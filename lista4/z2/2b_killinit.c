#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

int main() {
    printf("sending SIGKILL to pid 1\n");
    int kill_st = kill(1, SIGKILL);
    if (!kill_st)
    {
        printf("Signal send succesfully\n");
    } else {
        if (errno == EINVAL)
        {
            printf("invalid signal\n");
            exit(EXIT_FAILURE);
        }
        if (errno == EPERM)
        {
            printf("insufficent permissions\n");
            exit(EXIT_FAILURE);
        }
        if (errno == ESRCH)
        {
            printf("pid 1 doesn't exist");
            exit(EXIT_FAILURE);
        }
    }
    kill_st = kill(1, 0);
    if(kill_st == 0)
        printf("init still alive\n");
    else
        printf("init killed\n");
    
    return 0;
}
