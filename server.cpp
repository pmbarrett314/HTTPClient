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
char const *args[1];

void parse_arguments_and_flags(int argc, char *argv[]);

uint16_t get_port_from_args();

void handle_ctrl_c();

void create_socket();

void bind_socket(uint16_t port);

void listen_to_socket(int backlog);

void accept_client();

int communicate_with_client();

void disconnect_client();

int main(int argc, char *argv[])
{
    parse_arguments_and_flags(argc, argv);
    uint16_t port = get_port_from_args();
    handle_ctrl_c();

    printf("Started...\n");
    create_socket();
    bind_socket(port);

    int max_clients = 1;
    listen_to_socket(max_clients);
    printf("Listening...\n");

    int exitv = 0;
    do
    {
        accept_client();

        printf("Client connected...\n");
        exitv = communicate_with_client();

        disconnect_client();
        if (0 != exitv)
        {
            printf("client disconnected. listening...\n");
        }
        else
        {
            printf("client disconnected. ");
        }
    } while (0 != exitv);

    printf("closing...\n");
    close(sock);
    close(clientsock);
    exit(EXIT_SUCCESS);
}

void parse_arguments_and_flags(int argc, char *argv[])
{
    //parses argv and sets the value of args[0] to the port as a string
    extern char *optarg;
    extern int optind;
    int c;
    bool isPort = false;
    char const *defaultport = "4349";

    //parse flags
    while (-1 != (c = getopt(argc, argv, "d")))
    {
        switch (c)
        {
            case 'd':
                args[0] = defaultport;
                isPort = true;
                break;
            default:
                continue;
        }
    }

    //check that enough flags and arguments were given
    if (!isPort && ((argc - optind) == 0))
    {
        fprintf(stderr, "usage: %s serverport [-d]\nif -d is specified serverport is ignored\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    //fill in args with command line arguments if needed
    for (int i = optind; i < argc; i++)
    {
        switch (i)
        {
            case 1:
                args[0] = argv[i];
                break;
            default:
                continue;
        }
    }
}

uint16_t get_port_from_args()
{
    //returns the port, which is currently args[0], as a uint16_t
    //the argument parsing should be run first
    uint16_t port = 0;
    char const *portstring = args[0];
    if (0 == (port = validate_port(portstring, port)))
    {
        fprintf(stderr, "port not set correctly, input was: %s", portstring);
        exit(EXIT_FAILURE);
    }
    return port;
}

void handler(int sig)
{
    //handles cleanup when user presses ctrl+c
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
    //sets the ctrl+c handler properly
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
    //create our server socket
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

void bind_socket(uint16_t port)
{
    //bind the socket to the given port
    sockaddr_in serveraddr;

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
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
    //begin listening on the port with the given backlog of connections
    if (-1 == listen(sock, backlog))
    {
        perror("error listen failed");
        close(sock);
        exit(EXIT_FAILURE);
    }
}

void accept_client()
{
    //wait until client connects and accept
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
    //perform communication with client
    //receive data from the client, print it, then echo it back
    //check for flag values of ctrl+d or *QUIT* from the client
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
        if (strstr(buffer, "*QUIT*") != NULL)
        {
            printf("Client sent *QUIT*, terminating server...\n");
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
    //cleanup and close the client connection
    if (-1 == shutdown(clientsock, SHUT_RDWR))
    {
        perror("cannot shutdown socket");
        close(clientsock);
        close(sock);
        exit(EXIT_FAILURE);
    }
    close(clientsock);
}