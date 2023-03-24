#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>

#define BUF_SIZE 1024
#define HEADER_SIZE 12

void throw_error(char *message);

/* UDP Server Driver
 */
int main(int argc, char* argv[]) {
    // verify usage
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
    char response_header[] = "UDP Server ";

    // create socket
    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        throw_error("Failed to create socket");
    }
    printf("Server: socket built for port %s\n", argv[1]);

    // prepare server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(atoi(argv[1]));

    // bind socket to server address
    if (bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        throw_error("Failed to bind socket");
    }
    printf("Server: socket opened on port %s of localhost\n", argv[1]);

    while(1) {
        printf("Server: listening...\n");
        client_addr_size=sizeof(client_addr);

        // receive request from client
        recv_len = recvfrom(sock, message, BUF_SIZE, 0, (struct sockaddr*)&client_addr, &client_addr_size);
        printf("Server: Acepted client connection\n");
        printf("recv len : %d\n", recv_len);

        // ignore messages with invalid headers
        if(recv_len < HEADER_SIZE) {
            continue; 
        }
        printf("Received message from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        message[recv_len] = '\0';
        printf("%s\n",message);

        // extract header 
        char header[HEADER_SIZE+1];
        strncpy(header, message, HEADER_SIZE);
        header[HEADER_SIZE] = '\0';
        printf("Server: client header extracted: %s\n", header);

        // extract size and body
        char substr[recv_len-HEADER_SIZE+1];
        strncpy(substr, &message[HEADER_SIZE], recv_len);
        substr[recv_len] = '\0';

        // derive length of message
        char * token = strtok(substr, " ");
        printf("Server: client payload size extracted: %d\n", atoi(token)+1);

        char payload[atoi(token)+1]; // size of the body
        strncpy(payload, &message[HEADER_SIZE+2*strlen(token)], recv_len);
        printf("Server: client payload extracted: %s\n", payload);

        // convert payload to uppercase
        for(int i=0; i<strlen(payload); i++) {
            payload[i] = toupper(payload[i]);
        }
        payload[strlen(payload)] = '\0';

        // prepare response message
        char response[strlen(response_header)+strlen(payload)+1];
        strncpy(response, response_header, HEADER_SIZE);
        strcat(response, payload);
        response[HEADER_SIZE+strlen(payload)] = '\0';
        printf("Server: response structured: %s\n", response);

        // send response message to client
        sendto(sock, response, HEADER_SIZE+sizeof(int)+recv_len, 0, (struct sockaddr*)&client_addr, sizeof(client_addr));
        printf("Server: sent response to client\n");
    }
    close(sock);
    printf("Server: closing connection...\n");
    return 0;
}

/* Prints error message to console and exits
 */
void throw_error(char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
