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
char const *args[1];
const char *PORT = "80";
char host[2048];
char page[2048];

void handle_ctrl_c();

void parse_arguments_and_flags(int argc, char *argv[]);

void parseURL();

void create_socket();

void connect_to_server();

void send_request();

void handle_response();

int main(int argc, char *argv[]) {
    handle_ctrl_c();

    parse_arguments_and_flags(argc, argv);
    parseURL();

    printf("started with host: %s, page: %s ... \n", host,page);

    create_socket();

    connect_to_server();
    send_request();

    handle_response();

    close(sock);
    exit(EXIT_SUCCESS);
}

void parse_arguments_and_flags(int argc, char *argv[]) {
    //parse argv and set args[0] to the page URL
    int c;
    extern char *optarg;
    extern int optind;
    char const *defaultpage = "web.cse.msstate.edu/~ramkumar/cse4153s15.html";
    bool isPage = false;
    //parse flags
    while (-1 != (c = getopt(argc, argv, "d"))) {
        switch (c) {
            case 'd':
                args[1] = defaultpage;
                isPage = true;
                break;
            default:
                continue;
        }
    }

    //check that enough args and flags were given
    int argsleft = argc - optind;
    if (argsleft != 1 && !isPage) {
        fprintf(stderr, "usage: %s url [-d]\n\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    //fill in args using command line arguments if needed
    for (int i = optind; i < argc; i++) {
        if (!isPage) {
            //if there isn't a flag for page given, pick the first argument for page
            args[0] = argv[i];
            isPage = true;
        }

    }
}

void parseURL() {
    size_t first_slash = strcspn(args[0], "/");
    strncpy(host, args[0],first_slash);
    strncpy(page, &args[0][first_slash], strlen(args[0]) - first_slash);
}

void handler(int sig) {
    //handle cleanup on ctrl+c
    if (sig == SIGINT) {
        printf("closing...\n");
        close(sock);
        exit(EXIT_SUCCESS);
    }
}

void handle_ctrl_c() {
    //sets the ctrl+c handler properly
    struct sigaction sa;
    sa.sa_handler = handler;
    if (-1 == sigemptyset(&sa.sa_mask)) {
        perror("sigemptyset: ");
    }
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("error can't handle signal");
    }
}

void create_socket() {
    //create the socket
    if (-1 == (sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))) {
        perror("cannot create socket");
        exit(EXIT_FAILURE);
    }
}

void connect_to_server() {
    //connect to the server at the specified URL

    struct addrinfo hints;
    struct addrinfo *servinfo;
    memset(&hints,0,sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype=SOCK_STREAM;
    hints.ai_flags= AI_PASSIVE;

    int status;
    if ((status = getaddrinfo(host, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(EXIT_FAILURE);
    }

    if (-1 == connect(sock, servinfo->ai_addr, servinfo->ai_addrlen)) {
        close(sock);
        fprintf(stderr, "error connect failed: %s serverIP: %s port: %s\n", strerror(errno), args[0], PORT);
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(servinfo);

}

void send_request() {
    //sends the request to the server
    char message[5012];
    snprintf(message,5012,"GET %s HTTP/1.1\r\nHost: %s\r\n\r\n\r\n\0",page,host);
    send(sock,message,5012,0);

}

void handle_response()
{
    char recvbuffer[BUFSIZ];
    memset(recvbuffer,0,BUFSIZ);
    while(recv(sock, recvbuffer, BUFSIZ, 0)>0)
    {
        printf("%s", recvbuffer);
        memset(recvbuffer,0,BUFSIZ);
    }
}