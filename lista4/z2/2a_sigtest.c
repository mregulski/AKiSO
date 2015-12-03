#include <stdio.h>
#include <signal.h>
void handler(int signum)
{
    signal(signum, handler);
    printf("caught: %d\n",signum);
}
int main(int argc, char* argv[])
{  
    printf("test\n");
    for (int i=1; i<=64; i++ )
    {
        // signals 32 and 33 are not in kill -l 
        if(i!=32 && i!=33)
        {
            // register handler
            signal(i, handler);
            printf("signal(%d) = %d\n",i,signal(i, handler));
            // Raising these terminates the program, meaning they aren't handled. Uncomment to confirm.
            if(i!=9)
                raise(i);
        }
    }
    while(1);
    return 0;
}

