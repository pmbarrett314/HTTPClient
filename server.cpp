#include <signal.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <sys/unistd.h>
#include <stdlib.h>
#include "CSE4153.h"

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

int main(int argc, char *argv[])
{
    unsigned short port = 0;

    //Argument processing
    extern char *optarg;
    extern int optind;
    int c;
    bool isPort = false;
    while (-1 != (c = getopt(argc, argv, "d")))
    {
        switch (c)
        {
            case 'd':
                port = 4349;
                isPort = true;
                break;
            default:
                continue;
        }
    }

    int argsleft = argc - optind;
    if (argsleft == 0 && !isPort)
    {
        fprintf(stderr, "usage: %s serverport [-d]\nif -d is specified serverport is ignored\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    for (int i = optind; i < argc; i++)
    {
        switch (i)
        {
            case 1:
                if (0 == (port = validate_port(argv[i], port)))
                {
                    fprintf(stderr, "port not set correctly, input was: %s", argv[i]);
                    exit(EXIT_FAILURE);
                }
                break;
            default:
                continue;
        }
    }

    //Begin Connecting
    int backlog = 10;
    sockaddr_in serveraddr;


    printf("Started...\n");
    if (signal(SIGINT, sig_handler) == SIG_ERR)
    {
        perror("error can't handle signal");
    }


    if (-1 == (sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)))
    {
        perror("error socket not created");
        exit(EXIT_FAILURE);
    }

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    memset(&(serveraddr.sin_zero), 0, 8);

    int yes = 1;

    // lose the pesky "Address already in use" error message
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    {
        perror("setsockopt");
        exit(1);
    }

    if (-1 == bind(sock, (struct sockaddr *) &serveraddr, sizeof(serveraddr)))
    {
        perror("error bind failed");
        close(sock);
        exit(EXIT_FAILURE);

    }
    if (-1 == listen(sock, backlog))
    {
        perror("error listen failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("Listening...\n");
    do
    {
        sockaddr_in clientaddr;
        socklen_t addsize = sizeof(clientaddr);
        if (-1 == (clientsock = accept(sock, (sockaddr *) &clientaddr, &addsize)))
        {
            perror("error accept failed");
            close(sock);
            exit(EXIT_FAILURE);
        }
        printf("Client connected...\n");


        char buffer[BUFSIZ];
        while (0 < recv(clientsock, buffer, BUFSIZ, 0))
        {
            if (strchr(buffer, 4) != NULL)
            {
                printf("Client sent EOT. Disconnecting...\n");
                break;
            }
            printf("\"%s\" received \n", buffer);
            send(clientsock, buffer, strlen(buffer), 0);


        }

        if (-1 == shutdown(clientsock, SHUT_RDWR))
        {
            perror("cannot shutdown socket");
            close(clientsock);
            close(sock);
            exit(EXIT_FAILURE);
        }

        close(clientsock);
        printf("client disconnected. listening...\n");
    } while (18);
}

