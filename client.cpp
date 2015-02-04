#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/unistd.h>
#include <string.h>

#define BUFFERSIZE 500
int main()
{
    int sock;
    unsigned short port = 4349;
    char const *serverIP = "127.0.0.0";
    sockaddr_in serveraddr;

    if(-1==(sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))
    {
        return 1;
    }
    memset(&serveraddr,0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    inet_aton(serverIP, &serveraddr.sin_addr);
    if(-1==connect(sock, (struct sockaddr *) &serveraddr, sizeof(serveraddr)))
    {
        close(sock);
        return 2;
    }

    char buffer[BUFFERSIZE];
    strcasecmp(buffer, "Hello server!");
    send(sock, buffer, BUFFERSIZE, 0);

    close(sock);
    return 0;
}