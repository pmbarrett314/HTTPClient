#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <sys/unistd.h>

#define BUFFERSIZE 500

int main()
{
    printf("hello world");
    int sock, backlog = 10;
    sockaddr_in serveraddr;
    unsigned short port = 4349;
    if (-1 == (sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)))
    {
        return 1;
    }
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    memset(&(serveraddr.sin_zero), 0, 8);
    if (-1 == bind(sock, (struct sockaddr *) &serveraddr, sizeof(serveraddr)))
    {
        return 2;
        close(sock);
    }
    if (-1 == listen(sock, backlog))
    {
        close(sock);
        return 3;
    }
    int newsd, addsize;
    sockaddr_in clientaddr;
    do
    {
        if (-1 == (newsd = accept(sock, (sockaddr *) &clientaddr, &addsize)))
        {
            close(sock);
            return 4;
        }
        char buffer[BUFFERSIZE];
        while (0 < recv(newsd, buffer, BUFFERSIZE, 0))
        {
            printf(buffer);
        }

        close(newsd);
    } while (18);
    close(sock);
    return 0;
}