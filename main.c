#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>

int getAddressInfo(char host[], char port[], struct addrinfo **res)
{
    struct addrinfo hints;
    int errcode;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags |= AI_CANONNAME;

    errcode = getaddrinfo(host, port, &hints, res);
    if (errcode != 0)
    {
        perror("getaddrinfo");
        return -1;
    }

    return 0;
}

int main() {
    int socketfd, numBytesRecv, bufferSize = 6000;
    char buffer[bufferSize], port[] = "8081", host[] = "localhost";
    struct addrinfo *res;
    struct sockaddr_in socketaddr;

    getAddressInfo(host, port, &res);

    socketfd = socket(AF_INET, SOCK_STREAM, 0);

    socketaddr = *(struct sockaddr_in *)res->ai_addr;
    
    if (bind(
            socketfd,
            (struct sockaddr *)&socketaddr,
            sizeof(socketaddr)) == -1)
    {
        perror("bind");
        exit(1);
    }
    printf("Bind success\n");

    if (listen(socketfd, 10) == -1) {
        perror("listen");
        exit(1);
    }
    printf("Listen success\n");

    int session_fd = accept(socketfd, 0, 0);
    if (session_fd == -1)
    {
        perror("accept");
        exit(1);
    }

    numBytesRecv = recv(session_fd, &buffer, bufferSize, 0);
    if ( numBytesRecv == -1 ) {
        perror("recv");
        exit(1);
    }

    printf("Buffer: %s\n", buffer);
    
    return 0;
}