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
                serverIP = "127.0.0.1;";
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
    if ((argsleft == 0 && (!isPort || !isIP) || (argsleft == 1 && (!isPort && !isIP))))
    {
        fprintf(stderr, "usage: %s serverip serverport\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    for (int i = optind; i < argc; i++)
    {
        switch (i)
        {
            case 1:
                serverIP = argv[i];
                break;
            case 2:
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

    if (signal(SIGINT, sig_handler) == SIG_ERR)
    {
        perror("error can't handle signal");
    }
    printf("started...\n");

    sockaddr_in serveraddr;

    if (-1 == (sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)))
    {
        perror("cannot create socket");
        exit(EXIT_FAILURE);
    }
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    int result;
    if (0 >= (result = inet_pton(AF_INET, serverIP, &serveraddr.sin_addr)))
    {
        perror(0 > result ? "error: first parameter is not a valid address family" : "char string (second parameter does not contain valid ipaddress)");
        close(sock);
        exit(EXIT_FAILURE);
    }

    if (-1 == connect(sock, (struct sockaddr *) &serveraddr, sizeof(serveraddr)))
    {
        close(sock);
        perror("error connect failed");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFSIZ];

    while (1)
    {
        char *p;
        printf("Enter message, max size: %d, ctrl + d to quit\n", BUFSIZ);
        if (NULL == fgets(buffer, sizeof(buffer), stdin))
        {
            if (!ferror(stdin))
            {
                printf("EOF entered. disconnecting...\n");
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
        printints(buffer);
        send(sock, buffer, strlen(buffer) + 1, 0);

    }
    close(sock);
    exit(EXIT_SUCCESS);
}