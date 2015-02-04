#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFFERSIZE 500

int main(int argc, char *argv[])
{

    unsigned int port;
    char const *serverIP;
    int c;
    extern char *optarg;
    extern int optind;
    bool isPort = false, isIP = false;
    //Argument processing
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
                port = strtol(argv[i], NULL, 10);
                break;
        }
    }


    int sock;


    sockaddr_in serveraddr;

    if(-1==(sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)))
    {
        perror("cannot create socket");
        exit(EXIT_FAILURE);
    }
    memset(&serveraddr,0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    inet_aton(serverIP, &serveraddr.sin_addr);
    if(-1==connect(sock, (struct sockaddr *) &serveraddr, sizeof(serveraddr)))
    {
        close(sock);
        perror("error connect failed");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFERSIZE];
    strncpy(buffer, "Hello server!\n", 15);
    send(sock, buffer, BUFFERSIZE, 0);

    close(sock);
    return 0;
}