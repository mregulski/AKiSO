#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>

#define MAXMSG 2048
void emsg(char *msg)
{
    msg = (msg != NULL) ? msg : "";
    fprintf(stderr, "error %s\n", msg);
    //exit(EXIT_FAILURE);
}

void exit_msg(char *msg)
{
    emsg(msg);
    exit(EXIT_FAILURE);
}

// socket() wrapper for server: create & bind
// return: socket ready for server use
int server_tcp_socket(char *hostname, int port)
{
    struct sockaddr_in sn;
    int s;
    struct hostent *host_entry;        
    if(!(host_entry = gethostbyname(hostname)))
    {
        exit_msg("server_tcp_socket() > gethostname()");
    }
    bzero((char*)&sn, sizeof(sn));
    sn.sin_family = AF_INET; // InterNetwork
    sn.sin_port = htons((short)port);
    sn.sin_addr = *(struct in_addr *)(host_entry->h_addr_list[0]);
    if ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        exit_msg("server_tcp_socket() > socket()");
    if (bind(s, (struct sockaddr *)&sn, sizeof(sn)) == -1)
        exit_msg("server_tcp_socket() > bind()");
    return s;
}

// create sockets for each connection on server socket s
// return: new socket's file descriptor
int accept_tcp_connection(int s)
{
    struct sockaddr_in sn;
    socklen_t l = sizeof(sn); //how long is the address, for accept()
    int x;

    sn.sin_family = AF_INET;
    if(listen(s, 1) == -1) // try listening for one connection
        fprintf(stderr, "accept_tcp_connection() > listen()");
    // accept connection on socket s, return it's addres to sn
    x = accept(s, (struct sockaddr*)&sn, (socklen_t *)&l);
    if(x == -1)
        fprintf(stderr,"accept_tcp_connection() > accept()");
    return x;
}

// socket() wrapper for client: create and connect
// repeat until connected, with timeout
// return socket connected to a server @ hostname:port
#define req_timeout 2
int request_tcp_connection(char *hostname, int port)
{
    struct sockaddr_in sn;
    int s, ok, t;
    struct hostent *he;
    if(!(he = gethostbyname(hostname)))
        exit_msg("request_tcp_connection() > gethostbyname()");
    ok = 0;
    while(!ok)
    {
        sn.sin_family = AF_INET;
        sn.sin_port = htons((unsigned short)port);
        sn.sin_addr.s_addr = *(uint32_t *)(he->h_addr_list[0]);
        if ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1)
            exit_msg("request_tcp_connection() > socket()");
        ok = connect(s, (struct sockaddr *)&sn, sizeof(sn)) != -1;
        if (!ok)
        {
            sleep(1);
            t++;
            if ( t > req_timeout)
                return -1;
        }
    }
    return s;
}
