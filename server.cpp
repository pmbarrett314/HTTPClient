#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>

int main()
{
    printf("hello world");
    int sock, addsize, backlog = 10, newsd;
    sockaddr_in serveraddr, clientaddr;
    unsigned short port = 4349;
    if (sock = socket(AF_INET, SOCK_STREAM, 0) == -1)
    {
        return 1;
    }
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    memset(&(serveraddr.sin_zero), 0, 8);
    if (bind(sock, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) == -1)
    {
        return 2;
    }


    return 0;
}