#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <wait.h>

// FUNCTION PROTOTYPES
// +backend+
// main shell loop
void lsh_loop();
// read line from stdin
char *lsh_read_line();
// parse line into tokens
char **lsh_split_line(char *line);
// launch an external program
int lsh_launch(char **args);
// execute command (external program or builtin)
int lsh_execute(char **args);

// +builtins+
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);

// list of builit command names
char *builtin_str[] = {
    "cd",
    "help",
    "exit"
};
int (*builtin_func[])(char **) = {
    &lsh_cd,
    &lsh_help,
    &lsh_exit
};

// count builtins
int lsh_num_builtins() {
    return sizeof(builtin_str)/sizeof(char*);
}

// current child pid for handling SIGINT
pid_t pid;

// Signal handler for SIGINT.
// Forwards the signal to current fg process, if it exists.
// If no child exists, send it to itself.
void h_sigint(int signo)
{
    signal(SIGINT, h_sigint);
    if(pid>0)
    //printf("kill(%d,SIGINT)",pid);
    kill(pid, SIGINT);
}

// MAIN
int main(int argc, char **argv)
{
    // set up
    pid = getpid();
    signal(SIGINT, h_sigint);
    
    // main loop
    lsh_loop();
    
    // cleanup
}

// actual shell work
void lsh_loop()
{
    char *line;
    char **args;
    int status;
   // int pid;
    do
    {
        // TODO make handler
        pid = waitpid(-1, &status, WNOHANG);
        printf("%s> ",get_current_dir_name());
        line = lsh_read_line();
        args = lsh_split_line(line);
        status = lsh_execute(args);
    } while(status);
}

// get a line form stdin, no length limit
#define LSH_RL_BUFSIZE 1024
char *lsh_read_line()
{
    int bufsize = LSH_RL_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    //because eof is an int, not char
    int c;
    // make sure buffer got allocated
    if (!buffer)
    {
        fprintf(stderr, "[ERROR] lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }
    
    while(1)
    {
        c = getchar();
        // exit on Ctrl+D
        if(c==EOF)
        {
            printf("\n");
            exit(EXIT_SUCCESS);
        }
        // read until newline, replace it with null sign
        // (turn it into null-terminated string)
        if (/*c==EOF ||*/ c=='\n')
        {
            buffer[position] = '\0';
            return buffer;
        } else
        {
            buffer[position] = c;
        }
        position++;
        
        //extend buffer if needed
        if (position >= bufsize)
        {
            bufsize += LSH_RL_BUFSIZE;
            buffer = realloc(buffer,bufsize);
            if(!buffer)
            {
                fprintf(stderr, "[ERROR] lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}
//parse the command, tokens are space separated
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"
char **lsh_split_line(char *line)
{
    int bufsize = LSH_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(sizeof(char*) * bufsize);
    char *token;
    if(!tokens)
    {
        fprintf(stderr, "[ERROR] lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }
    //first token
    token = strtok(line, LSH_TOK_DELIM);
    while (token != NULL)
    {
        tokens[position] = token;
        position++;

        if (position >= bufsize)
        {
            bufsize += LSH_TOK_BUFSIZE;
            tokens = realloc(tokens, sizeof(char*) * bufsize);
            if(!tokens)
            {
                fprintf(stderr, "[ERROR] lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        //null because we use same string; strtok keeps a pointer to it
        token = strtok(NULL, LSH_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

// launch an outside program, use PATH
int lsh_launch(char **args)
{
    int background;
    char *lastarg;
    int status;
    //pid_t pid;
   // pid = waitpid(-1, &status, WNOHANG);
    /*if(pid>0)
        printf("waitpid reaped child pid=%d\n",pid);
    */
    //check if last arg is &
    int i = 0;
    while(args[i] != NULL)
        i++;
    lastarg = args[i-1];

    // if cmd ends in &, run it in background
    background = (strcmp(lastarg, "&")==0);
    if (background)
        args[i-1] = NULL;
    pid = fork();
    if(pid == 0) {
        // child
        if(background)
        {
           // fclose(stdout);
            //stdout = fopen("/dev/null","w");
        }
        //setpgid(0,0);
        if(execvp(args[0], args) == -1)
        {
            perror("lsh");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // fork error
        perror("lsh");
    } else {//parent 
        if(background)
        {
         //   tcsetpgrp(1, getpgid(0));
            //printf("running stuff in bg\n");
            waitpid(pid, &status, WNOHANG | WUNTRACED);
            
            
        }
        else
        {
           // printf("waiting\n");
            waitpid(pid, &status, 0);
        }
    }
    return 1;
}

int lsh_execute(char **args)
{
    int i;
    if (args[0] == NULL)
        return 1;

    // check if command is a builitn
    for (i = 0; i<lsh_num_builtins(); i++)
    {
        if (strcmp(args[0], builtin_str[i])==0)
            return (*builtin_func[i])(args);
    }

    // not a builitn, try executing as program
    return lsh_launch(args);
}
// BUILTINS
//implementations
int lsh_cd(char **args){
    if (args[1] == NULL) {
        fprintf(stderr, "lsh: expected argument to cd\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("lsh");
        }
    }
    return 1;
}
int lsh_help(char **args){
    // TODO display something
    return 1;
}

// simply returns 0, breaking the loop
int lsh_exit(char **args) {
    return 0;
}
