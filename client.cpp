int main()
{
    int sock;
    unsigned short port = 4349;
    char *serverIP = "127.0.0.0";
    sockaddr_in serveraddr;

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    inet_aton(serverIP, &serveraddr.sin_addr);
    connect(sock, (struct sockaddr *) &serveraddr, sizeof(serveraddr));

    return 0;
}