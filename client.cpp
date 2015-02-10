#include <signal.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "CSE4153.h"

int sock;

void sig_handler(int sig)
{
    if (sig == SIGINT)
    {
        printf("closing...\n");
        close(sock);
        exit(EXIT_SUCCESS);
    }
}

int main(int argc, char *argv[])
{

    uint16_t port = 0;
    char const *serverIP = "";

    //Argument processing
    int c;
    extern char *optarg;
    extern int optind;
    bool isPort = false, isIP = false;

    while (-1 != (c = getopt(argc, argv, "adl")))
    {
        switch (c)
        {
            case 'a':
                serverIP = "54.148.84.242";
                isIP = true;
                break;
            case 'l':
                serverIP = "127.0.0.1";
                isIP = true;
                break;
            case 'd':
                port = 4349;
                isPort = true;
                break;
            default:
                continue;
        }
    }
    int argsleft = argc - optind;
    if ((argsleft == 0 && (!isPort || !isIP)) || (argsleft == 1 && (!isPort && !isIP)))
    {
        fprintf(stderr, "usage: %s serverip serverport [-adl]\nif any flag is specified, the corresponding argument is ignored\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    for (int i = optind; i < argc; i++)
    {
        if (!isIP && (i == argsleft))
        {
            serverIP = argv[i];
            isIP = true;
        }
        else if (!isPort && (i == (argc - 1)))
        {
            if (0 == (port = validate_port(argv[i], port)))
            {
                fprintf(stderr, "port not set correctly, input was: %s\n", argv[i]);
                exit(EXIT_FAILURE);
            }
            isPort = true;
        }

    }

    if (signal(SIGINT, sig_handler) == SIG_ERR)
    {
        perror("error can't handle signal");
    }
    printf("started with IP: %s, port: %d... \n", serverIP, port);

    sockaddr_in serveraddr;

    if (-1 == (sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)))
    {
        perror("cannot create socket");
        exit(EXIT_FAILURE);
    }
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    int result = inet_pton(AF_INET, serverIP, &serveraddr.sin_addr);
    if (0 > result)
    {
        perror("error: first parameter is not a valid address family: %s\n");
        close(sock);
        exit(EXIT_FAILURE);
    }
    else if (0 == result)
    {
        fprintf(stderr, "invalid IP address, IP address entered was: %s\n", serverIP);
        close(sock);
        exit(EXIT_FAILURE);
    }

    if (-1 == connect(sock, (struct sockaddr *) &serveraddr, sizeof(serveraddr)))
    {
        close(sock);
        fprintf(stderr, "error connect failed: %s serverIP: %s port: %d\n", strerror(errno), serverIP, port);
        exit(EXIT_FAILURE);
    }

    char buffer[BUFSIZ];

    printf("Enter messages, max size: %d, ctrl + d to quit\n", BUFSIZ);
    while (1)
    {
        char *p;

        if (NULL == fgets(buffer, sizeof(buffer), stdin))
        {
            if (!ferror(stdin))
            {
                printf("EOT entered. disconnecting...\n");
                send(sock, "\x04\0", 2, 0);
                break;
            }
            else
            {
                perror("Input error : ");
                close(sock);
                exit(EXIT_FAILURE);
            }

        }

        else if ((p = strchr(buffer, '\n')) != NULL)
        {
            *p = '\0';
        }
        send(sock, buffer, strlen(buffer) + 1, 0);
        char recvbuffer[BUFSIZ];
        recv(sock, recvbuffer, BUFSIZ, 0);
        printf("server sent back: \"%s\"\n", recvbuffer);

    }
    close(sock);
    exit(EXIT_SUCCESS);
}