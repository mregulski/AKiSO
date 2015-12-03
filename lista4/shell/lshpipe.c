/*
 * lshpipe.c
 * LSH with pipe support 
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <wait.h>
#include <syscall.h>
#include <limits.h>

// debug is off by default
// actual debugger works better, but let's leave it for now
#ifndef DEBUG
#define DEBUG 0
#endif

// structure holding command information
typedef struct {
    int argc;
    char **args;
 } Command;

void lsh_loop();
// read line from stdin
char *lsh_read_line();
// parse line into tokens
Command **lsh_split_line(char *line);
// execute command (external program or builtin)
int lsh_execute(Command **cmds);
// launch multiple external programs piped together
int lsh_launch(Command **cmds);
// launch a single external program
int lsh_launch_single(Command *cmd, int background);
// check if command is a pipeline (contains '|')
int is_pipeline(char **args);

// +builtins+  
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);

// END PROTOTYPES

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
    if (pid>0)
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
    char *cwd = NULL;
    Command **cmds;
    int status;
    do
    {
        // reset current working dir, otherwise it alternates between actual directory and (null)
        cwd=NULL;
        pid = waitpid(-1, &status, WNOHANG);
        cwd = getcwd(cwd,0);
        printf("%s> ", cwd);
        line = lsh_read_line();
        cmds = lsh_split_line(line);
        status = lsh_execute(cmds);
        free(cwd);
        free(line);
        int i = 0;
        while(cmds[i] != NULL) {
            free(cmds[i]->args);
            cmds[i]->args=NULL;
            free(cmds[i]);
            cmds[i]=NULL;
            i++;
        }
        free(cmds);
        cmds=NULL;
    } while (status);
}

// get a line form stdin, no length limit
// replace with getline()?
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
    
    while (1)
    {
        c = getchar();
        // exit on Ctrl+D
        if (c==EOF)
        {
            printf("\n");
            exit(EXIT_SUCCESS);
        } 
        // read until newline, replace it with null sign
        // (turn it into null-terminated string)
        if (/*c==EOF ||*/ c=='\n') { 
            buffer[position] = '\0';
            return buffer;
        } else { 
            buffer[position] = c;
        }
        position++;
        
        //extend buffer if needed
        if (position >= bufsize) {
            bufsize += LSH_RL_BUFSIZE;
            buffer = realloc(buffer,bufsize);
            if (!buffer) {
                fprintf(stderr, "[ERROR] lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

//parse the command, tokens are space separated
#define LSH_CMDS_BUFSIZE 32
#define LSH_ARGS_BUFSIZE 64
#define LSH_TOK_PIPE_DELIM "|"
#define LSH_TOK_DELIM " \t\r\n\a"
// "constructor" for Command structs
void init_command(Command *cmd)
{
    cmd->argc = 0;
    cmd->args = malloc(sizeof(char*) * LSH_ARGS_BUFSIZE);
}

// parse line into LSH_TOK_PIPE_DELIM delimited commands, put them in a dynamic array of Command structs
Command **lsh_split_line(char *line)
{
    int bufsize_pipe = LSH_CMDS_BUFSIZE;
    int bufsize_args = LSH_ARGS_BUFSIZE;
    int line_i = 0, cmds_i = 0, args_i = 0;
    char **cmd_strings = malloc(sizeof(char*) * bufsize_pipe);
    char *cmd_string; 
    Command **cmds = malloc(sizeof(Command*) * bufsize_pipe);

    if (!cmd_strings) {
        fprintf(stderr, "[ERROR] lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    if (!cmds) {
        fprintf(stderr, "[ERROR] lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    // first pass: get commands in pipeline
    cmd_string = strtok(line, LSH_TOK_PIPE_DELIM);

    while (cmd_string != NULL) {
        cmd_strings[line_i] = cmd_string;
        line_i++;
        if (line_i >= bufsize_pipe) {
            bufsize_pipe += LSH_CMDS_BUFSIZE;
            cmd_strings = realloc(cmd_strings, sizeof(char*) * bufsize_pipe);
            if (!cmd_strings) {
                fprintf(stderr, "[ERROR] lsh: reallocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        cmd_string = strtok(NULL, LSH_TOK_PIPE_DELIM);
    }

    Command *cur_cmd;
    while (cmd_strings[cmds_i] != NULL) {
        cur_cmd = malloc(sizeof(Command));
        init_command(cur_cmd);
        cmd_string = strtok(cmd_strings[cmds_i], LSH_TOK_DELIM);
        args_i = 0;
        bufsize_args = LSH_ARGS_BUFSIZE;
        while (cmd_string != NULL) {
            cur_cmd->args[args_i] = cmd_string;
            cur_cmd->argc++;
            args_i++;
            if (args_i >= bufsize_args) {
                bufsize_args += LSH_ARGS_BUFSIZE;
                cur_cmd->args = realloc(cur_cmd->args, sizeof(char*) * bufsize_args);
                if (!cur_cmd->args) {
                    fprintf(stderr, "[ERROR] lsh: reallocation error\n");
                }
            }
            cmd_string = strtok(NULL, LSH_TOK_DELIM);
        }
        cmds[cmds_i] = cur_cmd;
        cmds_i++;
        if (cmds_i >= bufsize_pipe) {
            bufsize_pipe += LSH_CMDS_BUFSIZE;
            cmds = realloc(cmds, sizeof(Command*) * bufsize_pipe);
            if (!cmds) {
                fprintf(stderr, "[ERROR] lsh: reallocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
    free(cmd_string);
    //free(cmd_strings);
    //cmd_strings=NULL;
    return cmds;
}

// launch an outside program, use PATH
#define LSH_BG_INDICATOR "&"
int lsh_launch(Command **cmds)
{ 
    int background;
    char *lastarg;
    int status;

    int cmds_i = 0, args_i = 0;
    while  (cmds[cmds_i] != NULL) {
        args_i = 0;
        while (cmds[cmds_i]->args[args_i] != NULL)
            args_i++;
        cmds_i++;
    }
    int num_cmds = cmds_i;
    lastarg = cmds[cmds_i-1]->args[args_i-1];

    // if last arg of last command is LSH_BG_INDICATOR, set background execution for all commands
    background = (strcmp(lastarg, "&") == 0);
    if (background)
        cmds[cmds_i-1]->args[args_i-1] = NULL;

    args_i = 0;
    
    // ONE COMMAND            //
    // Execute without piping //
    ////////////////////////////
    if (num_cmds == 1)
        return lsh_launch_single(cmds[0], background);

    // MULTIPLE COMMANDS                                            //
    // Execute all as direct children of lsh and pipe them together //
    //////////////////////////////////////////////////////////////////

    // PIPE SETUP
    int inputs[num_cmds];
    int outputs[num_cmds];
    
    // init inputs and outputs to invalid descriptor value
    // -1 => standard stream will be used
    for (int a = 0; a < num_cmds; a++) {
        inputs[a] = -1;
        outputs[a] = -1;
    }

    // create a pipe for every pair of commands (1 less than number of commands)    
    for (int b = 0; b < num_cmds-1; b++) {
        int fds[2];
        if (pipe(fds) < 0) {
            perror("lsh: pipe");
            exit(EXIT_FAILURE);
        }
        // fds[0] is read, fds[1] is write
        // match n-th command's output to n+1's input
        inputs[b+1] = fds[0];
        outputs[b] = fds[1];
    }
    // END PIPE SETUP

    // FORK & EXEC

    // Pids of all child processess. Allows the shell to wait properly.
    pid_t cpid[num_cmds];
    // fork and exec, duplicate file descriptors as necessary for piping
    for (int c = 0; c < num_cmds; c++) {
        // Add next child's pid to the list
        cpid[c] = fork();
        switch (cpid[c]) {
            case -1:    // error
                {
                    perror("lsh: fork");
                    exit(EXIT_FAILURE);
                    break;
                }
            case 0:     // child
                {
                    
                    printf("command: %s \n", cmds[c]->args[0]);
                    // If output or input of current command is -1, leave it at default value 
                    // (stdin, stdout). Otherwise, close in or out stream and replace it with
                    // file descriptor from inputs or outputs array appropriately.
                    if (outputs[c] != -1) {
                        close(1);
                        dup2(outputs[c], 1);
                    }

                    if (inputs[c] != -1) {
                        close(0);
                        dup2(inputs[c], 0);
                    }

                    // exec() the command
                    if (execvp(cmds[c]->args[0], cmds[c]->args) == -1) {
                        perror("lsh: exec");
                        exit(EXIT_FAILURE);
                    }
                    break;
                }
            default:    // parent
                {
                    if (background) {
                        for(int pid_i = 0; pid_i < num_cmds; pid_i++)
                            waitpid(cpid[pid_i], &status, WNOHANG | WUNTRACED);
                    } else {   
                    // Close unnecessary file descriptors. Don't wait for children yet, because
                    // not all are created.
                    close(outputs[c]);
                    if (c > 0)
                        close(inputs[c]);
                    }
                }
        }
    }
    // Reap all the children.
    for(int pid_i = 0; pid_i < num_cmds; pid_i++)
        waitpid(cpid[pid_i], &status, 0);

    // commands aren't used anymore - free them
    /*int cmd_counter = 0;
    while(cmds[cmd_counter] != NULL)
    {
        free(cmds[cmd_counter]->args);
        free(cmds[cmd_counter]);
        cmd_counter++;
    }*/
    return 1;
}

// [✓] Launch a single command with args using execvp.
int lsh_launch_single(Command *cmd, int background)
{
    int status;
    
    pid = fork();
    switch (pid) {
        case -1:    // error
        {
            perror("lsh: fork");
            break;
        }
        case 0:     // child
        {
            if (execvp(cmd->args[0], cmd->args) == -1) {
                perror("lsh");
            }
            exit(EXIT_FAILURE);
            break;
        }
        default:    //parent
        {    
            if (background) {
                // non-blocking wait:
                // WNOHANG: returns to parent immediately
                // WUNTRACED: reports status of stopped child if it hasn't been reported yet
                // (reaps the child after it's done)
                waitpid(pid, &status, WNOHANG | WUNTRACED);
            } else { 
                // normal wait, hangs parent until child is done then reaps it's remains.
                waitpid(pid, &status, 0);
            }
        }
    }
    return 1;
}

int lsh_execute(Command **cmds)
{
    int i;
    if (cmds[0] == NULL)
        return 1;
    
    // check if _first_ command is a builitn
    // if it is, execute it and ignore the rest
    for (i = 0; i<lsh_num_builtins(); i++)
    {
        if (strcmp(cmds[0]->args[0], builtin_str[i]) == 0)
            return (*builtin_func[i])(cmds[0]->args);
    }

    // not a builitn, try executing s program
    return lsh_launch(cmds);
}
//////////////
// BUILTINS //
//////////////
//implementations
int lsh_cd(char **args)
{
    if (args[1] == NULL) {
        fprintf(stderr, "lsh: expected argument to cd\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("lsh");
        }
    }
    return 1;
}

int lsh_help(char **args)
{
    printf("LSH: simple shell\n"
            "To run command CMD in background: CMD &\n"
            "To connect stdout of CMD1 to stdin of CMD2 (pipe): CMD1 | CMD2\n"
            "builtins (don't support piping): \n"
            "cd DIR - change current directory to DIR\n"
            "help - show this message\n"
            "exit | ctrl-D - quit\n"
            );
    return 1;
}

// simply returns 0, breaking the loop
int lsh_exit(char **args)
{
    return 0;
}

// END builtins
int is_pipeline(char **args)
{
    int i = 0;
    while (args[i] != NULL)
    {
        if (strcmp(args[i],"|")==0)
            return 1;
        i++;
    }
}
