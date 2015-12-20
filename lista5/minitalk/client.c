#include "minitalk.h"

int main(int argc, char *argv[])
{
    char *hostname;
    int port, sock;
    fd_set fds;
    if(argc < 3)
        exit_msg("minitalk client\nusage: /path/client hostname port");
    hostname = argv[1];
    port = atoi(argv[2]);
    if(port < 5000 || port > 65535)
        exit_msg("port must be between 5000 and 65535 inclusive");

    if((sock = request_tcp_connection(hostname, port)) < 0)
        exit_msg("connection timed out");
    printf("connected\n");
    
    int st;
    char buf[MAXMSG];
    while(1)
    {   
        FD_ZERO(&fds);
        FD_SET(sock, &fds);
        FD_SET(0, &fds);
        st = select(sock+1, &fds, NULL, NULL, NULL);
        if (st < 0)
        {
            printf("error waiting for input.\n");
            perror("select");
            break;
        }
        if (FD_ISSET(0,&fds))
        {//input on stdin
            memset(buf,0,sizeof(buf));
            st = read(0, buf, sizeof(buf));
            if(st < 0)
            {
                printf("error reading stdin.\n");
                perror("read");
                continue;
            }
            st = write(sock, buf, sizeof(buf));
            if (st < 0)
            {
                printf("error writing to conenction/ \n");
                perror("write");
                break;
            }
        }
        if (FD_ISSET(sock, &fds))
        {
            memset(buf,0,sizeof(buf));
            st = read(sock, buf, sizeof(buf));
            if (st < 0)
            { 
                printf("error reading from connection.\n");
                perror("read");
                break;
            }
            if(buf[0] == '\0')
            {   
                printf("connection terminated by server\n");
                break;
            }
            printf(">%s\n", buf);
        }

    }
    close(sock);
    return 0;
}
