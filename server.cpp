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

char **parse_arguments_and_flags(int argc, char *argv[]);

unsigned short get_port_from_args(char **args);

void handle_ctrl_c();

void create_socket();

void bind_socket(unsigned short port);

void listen_to_socket(int backlog);

void accept_client();

int communicate_with_client();

void disconnect_client();

int main(int argc, char *argv[])
{
    char **args = parse_arguments_and_flags(argc, argv);
    unsigned short port = get_port_from_args(args);
    int max_clients = 1;
    handle_ctrl_c();
    printf("Started...\n");
    create_socket();
    bind_socket(port);
    listen_to_socket(max_clients);
    printf("Listening...\n");
    int exitv = 0;
    do
    {
        accept_client();
        printf("Client connected...\n");
        exitv = communicate_with_client();
        disconnect_client();
        printf("client disconnected. listening...\n");
    } while (0 != exitv);

    printf("closing...\n");
    close(sock);
    close(clientsock);
    exit(EXIT_SUCCESS);
}

char **parse_arguments_and_flags(int argc, char *argv[])
{
    //parses argv and returns an array of the finalized parameters as strings
    //return[0]=port
    char *returnval[1];
    extern char *optarg;
    extern int optind;
    int c;
    bool isPort = false;
    char *defaultport = "4349";
    while (-1 != (c = getopt(argc, argv, "d")))
    {
        switch (c)
        {
            case 'd':
                returnval[0] = defaultport;
                isPort = true;
                break;
            default:
                continue;
        }
    }

    int argsleft = argc - optind;
    if (!isPort && argsleft == 0)
    {
        fprintf(stderr, "usage: %s serverport [-d]\nif -d is specified serverport is ignored\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    for (int i = optind; i < argc; i++)
    {
        switch (i)
        {
            case 1:
                returnval[0] = argv[i];
                break;
            default:
                continue;
        }
    }
    return returnval;
}

unsigned short get_port_from_args(char **args)
{
    //returns the port, which is currently args[0]
    unsigned short port = 0;
    char *portstring = args[0];
    if (0 == (port = validate_port(portstring, port)))
    {
        fprintf(stderr, "port not set correctly, input was: %s", portstring);
        exit(EXIT_FAILURE);
    }
    return port;
}

void handler(int sig)
{
    if (sig == SIGINT)
    {
        printf("closing...\n");
        close(sock);
        close(clientsock);
        exit(EXIT_SUCCESS);
    }
}

void handle_ctrl_c()
{
    struct sigaction sa;
    sa.sa_handler = handler;
    if (-1 == sigemptyset(&sa.sa_mask))
    {
        perror("sigemptyset: ");
    }
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        perror("error can't handle signal");
    }
}

void create_socket()
{
    if (-1 == (sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)))
    {
        perror("error socket not created");
        exit(EXIT_FAILURE);
    }

    // lose the pesky "Address already in use" error message
    int yes = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    {
        perror("setsockopt");
        exit(1);
    }
}

void bind_socket(unsigned short port)
{
    sockaddr_in serveraddr;

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
}

void listen_to_socket(int backlog)
{
    if (-1 == listen(sock, backlog))
    {
        perror("error listen failed");
        close(sock);
        exit(EXIT_FAILURE);
    }
}

void accept_client()
{
    sockaddr_in clientaddr;
    socklen_t addsize = sizeof(clientaddr);
    if (-1 == (clientsock = accept(sock, (sockaddr *) &clientaddr, &addsize)))
    {
        perror("error accept failed");
        close(sock);
        exit(EXIT_FAILURE);
    }
}

int communicate_with_client()
{
    //returns 1 if valid, 0 if should quit
    int exitv = 1;
    char buffer[BUFSIZ];
    while (0 < recv(clientsock, buffer, BUFSIZ, 0))
    {
        if (strchr(buffer, 4) != NULL)
        {
            printf("Client sent EOT. Disconnecting...\n");
            break;
        }
        if (strstr(buffer, "*QUIT*") == NULL)
        {
            printf("Client sent *QUIT*, terminating server");
            exitv = 0;
            break;
        }
        printf("\"%s\" received \n", buffer);
        send(clientsock, buffer, strlen(buffer), 0);
    }
    return exitv;
}

void disconnect_client()
{
    if (-1 == shutdown(clientsock, SHUT_RDWR))
    {
        perror("cannot shutdown socket");
        close(clientsock);
        close(sock);
        exit(EXIT_FAILURE);
    }
    close(clientsock);
}