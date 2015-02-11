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
char const *args[2];

void parse_arguments_and_flags(int argc, char *argv[]);

struct in_addr get_IP_from_args();

uint16_t get_port_from_args();

void handle_ctrl_c();

void create_socket();

void connect_to_server(uint16_t port, struct in_addr serverIP);

void get_input_from_user(char *buffer);

int send_message_to_server(char *buffer);

int main(int argc, char *argv[])
{

    parse_arguments_and_flags(argc, argv);
    uint16_t port = get_port_from_args();
    struct in_addr serverIP = get_IP_from_args();
    printf("started with IP: %s, port: %d... \n", args[1], port);
    create_socket();
    connect_to_server(port, serverIP);
    printf("Connected...\nEnter messages, max size: %d, ctrl + d to quit, *QUIT* to kill server \n", BUFSIZ);

    int exitv = 0;
    do
    {
        char buffer[BUFSIZ];
        get_input_from_user(buffer);
        exitv = send_message_to_server(buffer);
        char recvbuffer[BUFSIZ];
        recv(sock, recvbuffer, BUFSIZ, 0);
        printf("server sent back: \"%s\"\n", recvbuffer);
    } while (0 != exitv);
    close(sock);
    exit(EXIT_SUCCESS);
}

void parse_arguments_and_flags(int argc, char *argv[])
{
    int c;
    extern char *optarg;
    extern int optind;
    bool isPort = false, isIP = false;
    char const *amazonIP = "54.148.84.242";
    char const *localhost = "127.0.0.1";
    char const *defaultport = "4349";

    while (-1 != (c = getopt(argc, argv, "adl")))
    {
        switch (c)
        {
            case 'a':
                args[0] = amazonIP;
                isIP = true;
                break;
            case 'l':
                args[0] = localhost;
                isIP = true;
                break;
            case 'd':
                args[1] = defaultport;
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
        if (!isIP && (i == optind))
        {
            args[0] = argv[i];
            isIP = true;
        }
        else if (!isPort && (i == (argc - 1)))
        {
            args[1] = argv[i];
            isPort = true;
        }
    }
}

struct in_addr get_IP_from_args()
{
    struct in_addr IP;

    int result = inet_pton(AF_INET, args[0], &IP);
    if (0 > result)
    {
        perror("error: first parameter is not a valid address family: %s\n");
        close(sock);
        exit(EXIT_FAILURE);
    }
    else if (0 == result)
    {
        fprintf(stderr, "invalid IP address, IP address entered was: %s\n", args[1]);
        close(sock);
        exit(EXIT_FAILURE);
    }
    return IP;
}

uint16_t get_port_from_args()
{
    //returns the port, which is currently args[0]
    uint16_t port = 0;
    char const *portstring = args[1];
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
        perror("cannot create socket");
        exit(EXIT_FAILURE);
    }
}

void connect_to_server(uint16_t port, struct in_addr serverIP)
{
    sockaddr_in serveraddr;

    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    serveraddr.sin_addr = serverIP;

    if (-1 == connect(sock, (struct sockaddr *) &serveraddr, sizeof(serveraddr)))
    {
        close(sock);
        fprintf(stderr, "error connect failed: %s serverIP: %s port: %d\n", strerror(errno), args[0], port);
        exit(EXIT_FAILURE);
    }

}

void get_input_from_user(char *buffer)
{
    if (NULL == fgets(buffer, BUFSIZ, stdin))
    {
        if (!ferror(stdin))
        {
            buffer[0] = 4;
            buffer[1] = '\0';
        }
        else
        {
            perror("Input error : ");
            close(sock);
            exit(EXIT_FAILURE);
        }
    }
}

int send_message_to_server(char *buffer)
{
    //returns 1 if valid, 0 if should quit
    int retval;
    if (strstr(buffer, "\x04\0") != NULL)
    {
        printf("EOT entered. disconnecting...\n");
        send(sock, buffer, strlen(buffer) + 1, 0);
        retval = 0;
    }
    else if (strstr(buffer, "*QUIT*") != NULL)
    {
        printf("*QUIT* entered. disconnecting and killing server...\n");
        send(sock, buffer, strlen(buffer) + 1, 0);
        retval = 0;
    }
    else
    {
        char *p;
        printints(buffer);
        if ((p = strchr(buffer, '\n')) != NULL)
        {
            *p = '\0';
            printf("thing");
        }
        send(sock, buffer, strlen(buffer) + 1, 0);
        retval = 1;
    }
    printints(buffer);
    return retval;
};