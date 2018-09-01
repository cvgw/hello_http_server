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

int find_char(char *buffer, char search_char) {
    char current_char = '0';
    int position = 0;

    while (current_char != '\0')
    {   
        current_char = buffer[position];
        if (current_char == search_char)
        {
            return position;
        }
        position++;
    }

    return -1;
}

void parseHeaders(char *currentBuff, char **headers, int headers_size, int *header_num) {
    int string_size, position = 0;
    char *header;
    while (*header_num < headers_size && currentBuff[0] != '\r')
    {
        position = find_char(currentBuff, '\n');
        if (position == -1)
            break;

        string_size = position - 1;
        headers[*header_num] = malloc(string_size);
        header = headers[*header_num];
        if (header == NULL)
        {
            perror("malloc");
            exit(1);
        }

        memcpy(header, currentBuff, string_size);
        header[string_size] = '\0';

        currentBuff = currentBuff + position + 1;
        (*header_num)++;
    }
}

void processBuffer(char *buffer)
{
    int string_size, position, i = 0, header_num = 0, headers_size = 10;
    char *operation, **headers;
    char *currentBuff;

    currentBuff = buffer;
    
    position = find_char(currentBuff, '\n');
    if (position == -1)
    {
        printf("couldn't find newline");
        exit(1);
    }

    currentBuff = currentBuff + position + 1;
    position = position - 1;
    
    operation = malloc(position);
    if (operation == NULL)
    {
        perror("malloc");
        exit(1);
    }
    memcpy(operation, buffer, position);
    operation[position] = '\0';    
    printf("Operation: %s\n", operation);

    headers = malloc(sizeof(char *)*headers_size);
    parseHeaders(currentBuff, headers, headers_size, &header_num);

    while (i < header_num)
    {
        printf("Header number %i: %s\n", i, headers[i]);
        free(headers[i]);
        i++;
    }


    free(headers);
    free(operation);
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
