#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>

#define BUF_SIZE 2048
#define HEADER_SIZE 12

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_size = sizeof(client_addr);
    char buffer[BUF_SIZE];
    char message[BUF_SIZE - HEADER_SIZE];
    int recv_len;

    // create socket
    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    // prepare server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(atoi(argv[1]));

    // bind socket to server address
    if (bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Failed to bind socket");
        exit(EXIT_FAILURE);
    }

    while(1) {
        client_addr_size=sizeof(client_addr);
        recv_len=recvfrom(sock, message, BUF_SIZE, 0,
                         (struct sockaddr*)&client_addr, &client_addr_size);
        if(recv_len < HEADER_SIZE) {
            continue; // ignore messages with invalid headers
        }

        // extract header and length information
        char header[HEADER_SIZE+1];
        strncpy(header, message, HEADER_SIZE);
        header[HEADER_SIZE] = '\0';
        int length = *(int*)(message+HEADER_SIZE);

        // extract payload
        char payload[length+1];
        strncpy(payload, message+HEADER_SIZE+sizeof(int), length);
        payload[length] = '\0';

        printf("Received message from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        printf("Header: %s\n", header);
        printf("Payload Length: %d\n", length);
        printf("Payload: %s\n", payload);

        // convert payload to uppercase
        for(int i=0; i<length; i++) {
            payload[i] = toupper(payload[i]);
        }

        // prepare response message
        char response[HEADER_SIZE+sizeof(int)+length+1];
        strncpy(response, header, HEADER_SIZE);
        *(int*)(response+HEADER_SIZE) = length;
        strncpy(response+HEADER_SIZE+sizeof(int), payload, length);
        response[HEADER_SIZE+sizeof(int)] = '\0';

        // send response message to client
        sendto(sock, response, HEADER_SIZE+sizeof(int)+length, 0,
               (struct sockaddr*)&client_addr, sizeof(client_addr));
    }

    close(sock);

    return 0;
}
