#include <stdio.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <semaphore.h>
#include <stdlib.h> //random()
#include <unistd.h>
#include <stdarg.h> // for thr_printf()

#ifndef N_PHIL
    #define N_PHIL 5 //how many philosophers
#endif
typedef struct {
    unsigned char id;
    const char *name;
    int my_forks;
} Philosopher;
// lock for thr_printf
static pthread_mutex_t printf_lock;
// 'thread-safe' printf: only one thread can use it at a time
void thr_printf(const char *format, ...);

static sem_t forks[N_PHIL]; //each semaphore = fork

#ifndef WAIT_MAX
    #define WAIT_MAX 5 
#endif
void think(const char *name)
{
    // in range [1..WAIT_MAX]
    int time = (random()%WAIT_MAX-1)+1;
    thr_printf("%s is thinking... [%ds]\n", name, time);
    sleep(time);
    thr_printf("%s is hungry\n", name);
}
void eat(const char *name)
{
    int time = random()%WAIT_MAX;
    thr_printf("%s is eating... [%ds]\n", name, time);
    sleep(time);
    thr_printf("%s finished eating\n", name);
}
// philosopher's thread logic
void *philosophize(void *philosopher)
{
    Philosopher *phil = (Philosopher*)philosopher;
    char left = phil->id%2 ;//so that all can't grab left/right fork at once
    while(1)
    {
        think(phil->name);
        //get forks;
        if(left)
        {
            sem_wait(&forks[phil->id]);
            sem_wait(&forks[(phil->id + 1) % N_PHIL]);
        }
        else
        { 
            sem_wait(&forks[(phil->id + 1) % N_PHIL]);
            sem_wait(&forks[phil->id]);
        }
        eat(phil->name);
        //leaveForks();
        sem_post(&forks[phil->id]);
        sem_post(&forks[(phil->id+1) % N_PHIL]);
    }
}
void check(void *ptr, char *msg);
int main(int argc, char *argv[])
{
    srandom(123456);
    char *names[] = { "\e[31mAristotle\e[0m", "\e[32mKant\e[0m", 
        "\e[33mSpinoza\e[0m", "\e[34mMarx\e[0m", "\e[35mRussell\e[0m",
        "\e[36mLocke\e[0m", "\e[37mPlato\e[0m", "\e[38mDescartes\e[0m", 
        "Confucius"};
    // make philosophers
    Philosopher *phils[N_PHIL];
    for(int i = 0; i < N_PHIL; i++)
    {
        sem_init(&forks[i], 0, 1);
        phils[i] = malloc(sizeof(Philosopher));
        check(phils[i], "allocation error");
        phils[i]->id = i;
        phils[i]->name = names[i];
        phils[i]->my_forks = 0;
    }
    // make threads, start
    pthread_mutex_init(&printf_lock, NULL);
    pthread_t threads[N_PHIL];
    for(int i = 0; i< N_PHIL; i++)
    {
        pthread_create(&threads[i], NULL, &philosophize, (void *)phils[i]);
    }
    sleep(10);
    printf("Dinner time's over\n");
    for(int i = 0; i < N_PHIL; i++)
    {
        pthread_cancel(threads[i]);
        free(phils[i]);
    }

    return 0;
}
void check(void *ptr, char *msg)
{
    if(msg == NULL)
        msg = "An error has occured\n";
    if(ptr == NULL)
    {
        fprintf(stderr, "%s\n", msg);
        exit(EXIT_FAILURE);
    }
}

void thr_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    pthread_mutex_lock(&printf_lock);
    vprintf(format, args);
    pthread_mutex_unlock(&printf_lock);
    va_end(args);
}
