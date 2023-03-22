#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024

void error_handling(char *message);

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in serv_addr;
    char message[BUF_SIZE];
    char header[] = "UDP Client 1";
    int message_len, total_len, header_len = strlen(header);
    int str_len, recv_len, recv_cnt;
    char recv_buf[BUF_SIZE];
    int i;

    if (argc != 3) {
        printf("Usage: %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock == -1)
        error_handling("socket() error");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    printf("Enter a message: ");
    fgets(message, BUF_SIZE, stdin);

    message_len = strlen(message) - 1; // exclude newline
    total_len = header_len + sizeof(int) + message_len;

    memcpy(message + header_len + sizeof(int), message, message_len); // move message to include length
    memcpy(message, header, header_len); // add header
    memcpy(message + header_len, &message_len, sizeof(int)); // add length of message

    sendto(sock, message, total_len, 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    recv_len = sizeof(serv_addr);
    recv_cnt = 0;
    while (recv_cnt < total_len) {
        str_len = recvfrom(sock, recv_buf, BUF_SIZE, 0, (struct sockaddr*)&serv_addr, &recv_len);
        if (str_len == -1)
            error_handling("recvfrom() error");
        recv_cnt += str_len;
        printf("Received packet %d bytes\n", str_len);
    }
    
    memcpy(&message_len, recv_buf + header_len, sizeof(int)); // extract message length
    memcpy(message, recv_buf + header_len + sizeof(int), message_len); // extract message
    
    printf("Received message from server: %s\n", message);

    close(sock);
    return 0;
}

void error_handling(char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
