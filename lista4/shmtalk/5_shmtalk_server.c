#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <string.h>
#include "5_shmtalk.h"
#include <pthread.h>
#include <time.h>
// SERVER
int shm_ids[MAX_CLIENTS*2];
int shm_ids_i = 0;
Message * prepare_shm(key_t mem_key)
{
    int mem_id;
    Message *msg;
    // set up shared memory
    if((mem_id = shmget(mem_key, sizeof(Message), IPC_CREAT | 0666)) < 0) {
        perror("shmtalk_server: shmget");
        exit(EXIT_FAILURE);
    }
    if((msg = (Message *) shmat(mem_id, NULL, 0)) == (void *) -1) {
        perror("shmtalk_server: shmat");
        exit(EXIT_FAILURE);
    }
    printf("mem_key: %d\tmem_id: %d\n", mem_key, mem_id);
    shm_ids[shm_ids_i] = mem_id;
    shm_ids_i++;
    // mark each fragment as uncalimed by any of the clients, so that they can claim them
    msg->client_id = -1;
    msg->status = EMPTIED;
    strcpy(msg->data, " ");
    return msg;
}
// global to allow deletion on SIGINT (server exit)
Message *read_mem[MAX_CLIENTS], *write_mem[MAX_CLIENTS];

// detach & remove shared memory on termination by SIGINT
void cleanup()
{
    signal(SIGINT, cleanup);
    for(int i = 0; i < MAX_CLIENTS; i++)
    {
        if (read_mem[i] != NULL)
        shmdt(read_mem[i]);
        if (write_mem[i] != NULL)
        shmdt(write_mem[i]);
    }
    for (int i = 0; i < MAX_CLIENTS * 2; i++)
    {
        shmctl(shm_ids[i], IPC_RMID, NULL);
    }
    printf("\nAll segments removed.\nServer will exit now.\n");
    exit(0);

}


int main(int argc, char *argv[])
{
    // create 2 memory segements for each possible client: 1 for reading 1 for writing
    // clients attach these segments in opposite roles (server's read = client's write)
    for (int i = 0; i < MAX_CLIENTS; i++)
       {
           read_mem[i] = prepare_shm(ftok("./5_shmtalk_server.c", 45 + i));
           write_mem[i] = prepare_shm(ftok("./5_shmtalk_client.c", 45 + i));
       }
    printf("Memory assigned properly.\n"); 
    signal(SIGINT, cleanup);
    int i = 0;
    struct timespec t = {0, 50000};
    while (1)
    { 
        for (i = 0; i < MAX_CLIENTS; i++)   
        { 
            //wait until something is written into read part
            if (read_mem[i]->status == EMPTIED)
                continue;
            for(int j = 0; j< MAX_CLIENTS; j++)
            { 
                // don't echo stuff to the client that sent it
                if(j!=i && strcmp(read_mem[i]->data,""))
                { 
                    strcpy(write_mem[j]->data, read_mem[i]->data);
                    write_mem[i]->status = FULL;
                    read_mem[i]->status = EMPTIED;
                }
            }
        }
        // sleep a tiiiiiiny bit to reduce processor usage but keep response almost instant
        nanosleep(&t,NULL); 
    }
    return 0;
} 

