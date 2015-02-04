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

int main(int argc, char *argv[])
{
    unsigned short port = 4349;
    int c;

    extern char *optarg;
    extern int optind;
    //Argument processing
    while (-1 != (c = getopt(argc, argv, "")))
    {
    }
    if (argc - optind < 1)
    {
        fprintf(stderr, "usage: %s serverport\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    for (int i = optind; i < argc; i++)
    {
        switch (argc - optind)
        {
            case 1:
                port = strtol(argv[i], NULL, 10);
                break;
        }
    }

    printf("Started...\n");
    if (signal(SIGINT, sig_handler) == SIG_ERR)
    {
        perror("error can't handle signal");
    }
    int backlog = 10;
    sockaddr_in serveraddr;

    if (-1 == (sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)))
    {
        perror("error socket not created");
        exit(EXIT_FAILURE);
    }
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    memset(&(serveraddr.sin_zero), 0, 8);
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
    socklen_t addsize;
    sockaddr_in clientaddr;
    printf("Listening...\n");
    do
    {
        if (-1 == (clientsock = accept(sock, (sockaddr *) &clientaddr, &addsize)))
        {
            perror("error accept failed");
            close(sock);
            exit(EXIT_FAILURE);
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

