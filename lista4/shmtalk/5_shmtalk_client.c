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
// CLIENT
Message * prepare_shm(key_t mem_key, int id)
{
    int mem_id;
    Message *msg;

    // set up shared memory
    if((mem_id = shmget(mem_key, sizeof(Message), 0666)) < 0) {
        perror("shmtalk_server: shmget");
        exit(EXIT_FAILURE);
    }

    if((msg = (Message *) shmat(mem_id, NULL, 0)) == (void *) -1) {
        perror("shmtalk_server: shmat");
        exit(EXIT_FAILURE);
    }
    // only give the memory to caller if it's not claimed bya nother process
    if (msg->client_id == -1)        
    {
        msg->client_id = id;
        msg->status = EMPTIED;
        return msg;
    }
    // = keep looking
    return NULL;
}

void *listen(void *read_mem)
{ 
    Message *read_msg = (Message *) read_mem;
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = 400000;
    // check if server has something to say, forever
    while(1)
    { 
        if(strcmp(read_msg->data, ""))
        {    
            printf(">%s\n", read_msg->data);
            strcpy(read_msg->data, "");
            read_msg->status = EMPTIED;
        }
    }
}

// global, because we need to detach them on SIGINT
Message *read_mem = NULL, *write_mem = NULL;

// Detach memory on termination by SIGINT, but don't remove it
// so that a new client may connect.
// Segments are removed by server when it receives SIGINT.
void cleanup()
{  
    signal(SIGINT, cleanup);
    // So that the listener doesn't spew trash in infinite loop.
    // We'd have to quit that other thread anyway.
    read_mem->client_id = -1;
    shmdt(read_mem);
    write_mem->client_id = -1;
    shmdt(write_mem);
    exit(0);
}


int main()
{ 
 
    int i = 0;
    // Get the first read and write memory segment that is not claimed by another process
    while(read_mem == NULL || write_mem == NULL)
    { 
        read_mem = prepare_shm(ftok("./5_shmtalk_client.c", 45+i), i);
        write_mem = prepare_shm(ftok("./5_shmtalk_server.c", 45+i), i);
        i++;
        if(i > MAX_CLIENTS)
        {
            fprintf(stderr, "All server slots are currently taken\n");
            exit(EXIT_FAILURE);
        }
                    
    }
    
    signal(SIGINT, cleanup);
    strcpy(read_mem->data, "");
    pthread_t listener;
    pthread_create(&listener, NULL, listen, (void *) read_mem);
    char *line = NULL;
    size_t bufsize;
    int a = 0;
    while(1)
    {
        getline(&line, &bufsize, stdin);
        while(line[a] != '\n' && line[a] != '\0')
        {
            a++;
            if (line[a] == '\n')
                line[a] = '\0';
        }
        a = 0;

        if(strlen(line) < 1024)
        {
            strcpy(write_mem->data, line);
            if(strcmp(read_mem->data," ") || strcmp(read_mem->data,"") || read_mem->data[0] != '\0');
                printf("%s",read_mem->data);
            write_mem->status = FULL;
            read_mem->status = EMPTIED;
        }
    }

}

