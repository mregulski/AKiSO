#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

void *make_thread()
{
    pthread_t t2;
    printf("tid %d: creating new thread\n", pthread_self());
    if (!pthread_create(&t2, NULL, NULL, NULL))
        printf("tid %d: created thread: %d\n", pthread_self(), t2);
    pthread_exit(NULL);
}

int main()
{
    pthread_t t1;
    printf("main: creating new thread\n", pthread_self());
    if(!pthread_create(&t1, NULL, make_thread, NULL))
        printf("main: created thread: %ld\n", t1);
    void *status = NULL;
    pthread_join(t1, NULL);
    return 0;
}
