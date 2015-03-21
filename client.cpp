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
const char * port = "80";
char host[2048];
char page[2048];

void parse_arguments_and_flags(int argc, char *argv[]);

struct in_addr get_IP_from_args();

void handle_ctrl_c();

void create_socket();

void connect_to_server();

void parseURL();

void get_input_from_user(char *buffer);

int send_message_to_server(char *buffer);

int main(int argc, char *argv[]) {
    parse_arguments_and_flags(argc, argv);

    handle_ctrl_c();

    printf("started with URL: %s ... \n", args[0]);
    parseURL();
    exit(EXIT_SUCCESS);
    create_socket();

    connect_to_server();
    printf("Connected...\nEnter messages, max size: %d, ctrl + d to quit, *QUIT* to kill server \n", BUFSIZ);

    int exitv = 0;
    do {
        char buffer[BUFSIZ];
        get_input_from_user(buffer);

        exitv = send_message_to_server(buffer);
        if (0 != exitv) {
            //get return from the server and print it
            char recvbuffer[BUFSIZ];
            recv(sock, recvbuffer, BUFSIZ, 0);
            printf("server sent back: \"%s\"\n", recvbuffer);
        }
    } while (0 != exitv);

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
    printf("%s\t\t%s", host, page);
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

    int status;
    if ((status = getaddrinfo(args[0], port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }

    sockaddr_in serveraddr;

    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    //serveraddr.sin_port = htons(port);
    //serveraddr.sin_addr = serverIP;

    if (-1 == connect(sock, (struct sockaddr *) &serveraddr, sizeof(serveraddr))) {
        close(sock);
        fprintf(stderr, "error connect failed: %s serverIP: %s port: %d\n", strerror(errno), args[0], port);
        exit(EXIT_FAILURE);
    }

}

void get_input_from_user(char *buffer) {
    //get input from commandline and check for ctrl+d
    if (NULL == fgets(buffer, BUFSIZ, stdin)) {
        if (!ferror(stdin)) {
            buffer[0] = 4;
            buffer[1] = '\0';
        }
        else {
            perror("Input error : ");
            close(sock);
            exit(EXIT_FAILURE);
        }
    }
}

int send_message_to_server(char *buffer) {
    //checks the user input to see if it should quit
    //then sends the data
    //returns 1 if should continue, 0 if should quit
    int retval;
    if (strstr(buffer, "\x04\0") != NULL) {
        printf("EOT entered. disconnecting...\n");
        send(sock, buffer, strlen(buffer) + 1, 0);
        retval = 0;
    }
    else if (strstr(buffer, "*QUIT*") != NULL) {
        printf("*QUIT* entered. disconnecting and killing server...\n");
        send(sock, buffer, strlen(buffer) + 1, 0);
        retval = 0;
    }
    else {
        //remove trailing newline and send data.
        char *p;
        if ((p = strchr(buffer, '\n')) != NULL) {
            *p = '\0';
        }
        send(sock, buffer, strlen(buffer) + 1, 0);
        retval = 1;
    }
    return retval;
};