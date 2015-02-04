#include <signal.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <sys/unistd.h>
#include <stdlib.h>

#define BUFFERSIZE 500

int sock, clientsock;

void sig_handler(int sig)
{
    if (sig == SIGINT)
    {
        printf("closing...\n");
        close(sock);
        close(clientsock);
        exit(EXIT_SUCCESS);
    }
}

int main()
{
    printf("Started...\n");
    if (signal(SIGINT, sig_handler) == SIG_ERR)
    {
        printf("Can't handle signal. \n");
    }
    int backlog = 10;
    sockaddr_in serveraddr;
    unsigned short port = 4349;
    if (-1 == (sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)))
    {
        return 1;
    }
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    memset(&(serveraddr.sin_zero), 0, 8);
    if (-1 == bind(sock, (struct sockaddr *) &serveraddr, sizeof(serveraddr)))
    {
        return 2;
        close(sock);
    }
    if (-1 == listen(sock, backlog))
    {
        close(sock);
        return 3;
    }
    socklen_t addsize;
    sockaddr_in clientaddr;
    printf("Listening...\n");
    do
    {
        if (-1 == (clientsock = accept(sock, (sockaddr *) &clientaddr, &addsize)))
        {
            close(sock);
            return 4;
        }
        printf("Client connected...\n");

        char buffer[BUFFERSIZE];
        while (0 < recv(clientsock, buffer, BUFFERSIZE, 0))
        {
            printf(buffer);
        }

        close(clientsock);
    } while (18);
    close(sock);
    return 0;
}

