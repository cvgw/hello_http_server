#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>

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

void processBuffer(char *buffer)
{
    int currentBuffLen, i = 0;
    char *operation, *header, **headers;
    const char *currentBuff;
    const char *nextBuff;

    currentBuff = buffer;

    nextBuff = strchr(currentBuff, '\n');
    currentBuffLen = (nextBuff - currentBuff);
    operation = malloc(currentBuffLen);
    if (operation)
    {
        memcpy(operation, currentBuff, currentBuffLen);
        printf("Operation: %s\n", operation);
    }
    else
    {
        perror("malloc");
        exit(1);
    }

    currentBuff = nextBuff + 1;

    headers = malloc(sizeof(char *) * 10);
    if (headers == NULL)
    {
        perror("malloc");
        exit(1);
    }
    i = 0;

    while (currentBuff)
    {
        nextBuff = strchr(currentBuff, '\n');
        currentBuffLen = (nextBuff - currentBuff);
        header = malloc(currentBuffLen + 1);
        if (header)
        {
            memcpy(header, currentBuff, currentBuffLen);
            header[currentBuffLen] = '\0';
            if (strcmp(header, "\r") == 0)
            {
                break;
            }
            printf("Header %d: %s\n", i, header);
            *(headers + i) = header;
        }
        else
        {
            perror("malloc");
            exit(1);
        }
        currentBuff = nextBuff + 1;
        i++;
        free(header);
    }

    currentBuff = nextBuff + 1;

    int buffLen = strlen(currentBuff);
    char *body = malloc(buffLen + 1);
    if (body)
    {
        memcpy(body, currentBuff, buffLen);
        body[buffLen] = '\0';
        printf("Body: %s\n", body);
    }
    else
    {
        perror("malloc");
        exit(1);
    }

    free(operation);
    free(header);
    free(headers);
    free(body);
}

int main() {
    int socketfd, session_fd, numBytesRecv, bufferSize = 6000;
    char buffer[bufferSize], port[] = "8081", host[] = "localhost";
    struct addrinfo *res;
    struct sockaddr_in *socketaddr;

    getAddressInfo(host, port, &res);

    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd == -1) {
        perror("socket");
        exit(1);
    }

    socketaddr = (struct sockaddr_in *)res->ai_addr;
    
    if (bind(socketfd, (struct sockaddr *)socketaddr, sizeof(*socketaddr)) == -1) {
        perror("bind");
        exit(1);
    }
    printf("Bind success\n");

    if (listen(socketfd, 10) == -1) {
        perror("listen");
        exit(1);
    }
    printf("Listen success\n");

    session_fd = accept(socketfd, 0, 0);
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

    processBuffer(buffer);

    if (close(session_fd) != 0)
    {
        perror("close");
    }
    
    if (close(socketfd) != 0)
    {
        perror("close");
    }

    return 0;
}
