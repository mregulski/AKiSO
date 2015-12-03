#include <stdio.h>

int main(int argc, char *argv[])
{
    int attr = 0;
    int fg = 0;
    int bg = 0;
    int i = 0;
    printf("Standard escape sequences\n");
    
    for(bg = 0; bg <= 8; bg++)
    {
        for(attr=0; attr <= 8; attr++)
        {
            printf("\x1B[%d;%d;%dm   TEST   ", attr, fg+30, bg+40);
//            printf("Test(a: %d, f:%d, b: %d)", attr, fg, bg);
            i++;
            if(i>0 && i%8==0)  printf("\x1B[39;49m\n");
        }
    }
    
    printf("\x1B[0;39;49m\n\n256color mode\n");
    i = 0;
    for(fg = 0; fg < 256; fg++)
    {
        printf("\033[48;5;%dm   TEST   ",fg);
        i++;
        if (i>0 && i%8==0) printf("\x1B[39;49m\n");
    }
    printf("\x1B[39;49m\n");
    return 0;            
}
