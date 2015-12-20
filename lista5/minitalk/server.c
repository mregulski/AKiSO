#include "minitalk.h"
#include <sys/select.h>
#include <signal.h>


#define MAX_CON 10
#define UN_LEN 30
#define __END__ /* }}}}}}}} */
typedef struct {
    int fd;
    char *name;
 } User;

static User *users[MAX_CON];        // all connected & logged in users
static unsigned int usr_count = 0;  // current number of  users
fd_set active_fd_set, read_fd_set;  // global for cleanup()
                                    // MUST __NOT__ BE STATIC!
static int srv_sock;
// PROTOTYPES
// user control
static User *new_user(int fd);  // create a new user on given socket
int find_user_fd(int fd);       // get idx of user with given socket
int find_user(char *name);      // get idx of user with given name
char *list_users();             // lists usernames
int user_exit(User *usr);       // remove the user at idx and adjust list

// socket I/O
char *client_read(int filedes); // read from slient socket
int client_write(int filedes, char *msg);   // write msg to socket

// message handling
void handle_input(User *sender, char *message); // decide what to do with 

// cleanup function
void cleanup();
// END PROTOTYPES
//
int main(int argc, char *argv[])
{
    signal(SIGINT, cleanup);
    char *hostname;
    int port;
    //fd_set active_fd_set, read_fd_set;
    
    // check arguments
    if (argc < 3)
        exit_msg("minitalk server\nusage: /path/server hostname port");
    hostname = argv[1];
    port = atoi(argv[2]);
    if(port < 5000 || port > 65535)
        exit_msg("port must be between 5000 and 65535 inclusive"); 
    
    // init set of sockets
    srv_sock = server_tcp_socket(hostname, port);
    FD_ZERO(&active_fd_set);
    FD_SET(srv_sock,&active_fd_set); 
    
    int new_fd;
    char msg[MAXMSG+1],buf[MAXMSG+1];
    int cont = 1;
    while(cont)
    {
        read_fd_set = active_fd_set;
        // wait until there's something to read
        if (select (FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0)
            exit_msg("select()");

        // handle active sockets
        for (int fd = 0; fd <= FD_SETSIZE; fd++)
        {
            if(FD_ISSET(fd, &read_fd_set))
            { 
                /*if (fd == 0)
                {//stdin
                    char *tmp = client_read(0);
                    if(tmp != NULL && strncmp(tmp, "quit", 4)==0)
                    {
                        cont = 0;   // break while
                        break;      // break to while
                    }
                }
                else if (fd == 1)
                {// stdout
                    char *tmp = client_read(fd);
                    write(0, tmp, strlen(tmp));
                }*/
                if (fd == srv_sock)
                {// something to read on server socket
                    new_fd = accept_tcp_connection(srv_sock);
                    if (new_fd < 0)
                    {
                        emsg("error accepting new connection");
                        cont = 0;
                        break;
                    }

                    if (usr_count < MAX_CON)
                    {
                        fprintf(stderr, "Server: new connection\n");
                        users[usr_count] = new_user(new_fd);
                        usr_count++;
                        // add new fd to be listned to
                        FD_SET(new_fd, &active_fd_set);
                    }
                    else
                    {// too many clients
                        sprintf(msg, "Sorry, too many clients."
                                "Try again later\n");
                        write(new_fd, msg, strlen(msg));
                        close(new_fd);
                    }
                }
                else
                {//data on existing socket
                    int usr_id = find_user_fd(fd);
                    char *tmp = client_read(fd);
                    if(tmp == NULL || tmp[0]=='\0')
                    {
                        printf("client [#%d]: connection dropped\n", usr_id);
                        user_exit(users[usr_id]); 
                        continue;
                    }
                    strncpy(buf, tmp ,MAXMSG+1);
                    if(buf == NULL)
                    {//connection droppped/closed
                        user_exit(users[usr_id]);
                        //FD_CLR(fd,&active_fd_set);
                    }
                    else
                        handle_input(users[usr_id], buf);
                    //memset(buf, 0, sizeof(buf));
                }
            } // ENDIF

        } // ENDFOR

     } //ENDWHILE
    cleanup();
} __END__

// logic for message handling
void handle_input(User *sender, char *message)
{
    if(strncmp(message, "/login", 6) == 0) //message starts with "/login"
    {//logging in: create new user if name is unique
        
        char usrname[UN_LEN];
        strncpy(usrname, message+7, UN_LEN);
        if(sender->name[0] != '\0') //User->name is inited to 0's
        {// duplicate username
            client_write(sender->fd, "you're already logged in");
            return;
        }
        else if(find_user(usrname) >= 0)
        {
            client_write(sender->fd, "this user already exists");
            return;
        }
        else
        {
            strncpy(sender->name,usrname, UN_LEN);
        }
        // logged in successfully
        char response[UN_LEN+9];

        snprintf(response, UN_LEN+9,"Hello, %s\n", sender->name);
        printf("user logged in: '%s'\n", sender->name);
        fflush(stdout);
        client_write(sender->fd, response);
        client_write(sender->fd, "Logged in users:\n-----------------\n");
        client_write(sender->fd, list_users(sender));
        client_write(sender->fd, "====================================\n");
        client_write(sender->fd, "commands: /list, /logout\n"
             "<username> <message>: private message to user <username>\n");
    }
    else if(sender->name[0] == '\0') // not logged in
    {
        client_write(sender->fd, "use /login <name> to login");
        return;
    }
    else if(strncmp(message, "/logout", 7) == 0)
    {//logging out: remove user
       user_exit(sender); 
    }
    else if(strncmp(message, "/list", 5) == 0)
    {
        client_write(sender->fd, "Logged in users:\n-----------------\n");
        client_write(sender->fd, list_users(sender));
        client_write(sender->fd, "====================================\n");
    }
    else // logged in
    {
        //message format: <receiver> <message>
        char recv_name[UN_LEN+1] = {0};//strtok(message, " \t");
        for (int i = 0; i < UN_LEN+1; i++)
        {
            if(message[i] == ' ')
                break;
            recv_name[i] = message[i];
        }
        //strncpy(recv_name, message, UN_LEN+1);
        int recvid = find_user(recv_name);
        if(recvid < 0)
        {//receiver not found
            char resp[UN_LEN+23];
            snprintf(resp, UN_LEN+23,"user '%s' not logged in", recv_name);
            client_write(sender->fd, resp);
            return;
        }
        User *receiver = users[recvid];
        char msgtosend[MAXMSG+UN_LEN+4];
        snprintf(msgtosend, MAXMSG+UN_LEN+3, "[%s] %s", 
                    sender->name, message+strlen(recv_name)+1);
        client_write(receiver->fd, msgtosend);
    }
} __END__

static User *new_user(int fd)
{
    User *usr = malloc(sizeof(User));
    usr->fd = fd;
    usr->name = calloc(UN_LEN+1, sizeof(char));
    return usr;
} __END__

char *list_users(User *caller)
{
    //list = MAX_CON*(UN_LEN<username> + "\n") + "\0"
    char *list = calloc(UN_LEN*MAX_CON+MAX_CON+1, sizeof(char));
    for(int i = 0; i < MAX_CON; i++)
    {
        if(users[i] != NULL && users[i] != caller)
        {
            strncat(list, users[i]->name,UN_LEN);
            strcat(list, "\n");
        }

    }
    if (list[0] == '\0')
        strcpy(list, "<there's no one here>\n");
    return list;
}

// delete user, and adjust the list accordingly
int user_exit(User *usr)
{ 
    // get user id on list for pushback
    int i = 0;
    for(i = 0; i < MAX_CON; i++)
    {
        if (users[i] == usr)
            break;
    }
    printf("User logging out: [#%d]\n", i);
    if(usr == NULL)
        return -1;
    FD_CLR(usr->fd, &active_fd_set);
    close(usr->fd);
    free(usr);
    // move users after removed one 1 position back
    for(;i < MAX_CON - 1; i++)
        users[i] = users[i+1];
    usr_count--;
    return 0;
} __END__

char *client_read(int filedes)
{ 
    char *buffer = calloc(MAXMSG, sizeof(char));
    int nbytes;
    nbytes = read(filedes, buffer, MAXMSG);
    if (nbytes < 0) // read error
        exit_msg("client_read() > read()");
    else if (nbytes == 0) //EOF: connection closed
        return NULL;
    else // data read
    {
        buffer[strlen(buffer)-1]='\0'; //remove final newline
        //fprintf(stderr, "server: got msg: '%s'\n", buffer);
        return buffer;
    }
    return NULL; //not reachable, prevents warning

} __END__

int client_write(int filedes, char *msg)
{
    return write(filedes, msg, strlen(msg));
} __END__

int find_user(char *name)
{
    for(int i = 0; i < MAX_CON; i++)
        if(users[i] != NULL)
            if(users[i]->name != NULL)
                if(strcmp(users[i]->name, name) == 0)
                    return i;
    return -1;
} __END__

int find_user_fd(int fd)
{
    for (int i = 0; i < MAX_CON; i++)
        if(users[i] != NULL)
            if(users[i]->fd == fd)
                return i;
    return -1;
} __END__


void cleanup(int signo)
{
    // notify all users about shutdown
    for(int i = 0; i < MAX_CON; i++)
    {
        if(users[i]!=NULL)
        {
            client_write(users[i]->fd, "Server shutting down");
            FD_CLR(users[i]->fd, &active_fd_set);
            close(users[i]->fd);
            free(users[i]->name);
            free(users[i]);
        }
    }
    close(srv_sock);
    printf("Received signal(%d).\nShutting down...\n", signo);
    fflush(stdout);
    exit(EXIT_SUCCESS);
}

