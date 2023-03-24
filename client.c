#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024

void throw_error(char *message);

/* UDP Client Driver
*/
int main(int argc, char *argv[]) {
    // verify usage
    if (argc != 3) {
        printf("Usage: %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    int sock;
    struct sockaddr_in serv_addr;
    char message[BUF_SIZE];
    char formatted_message[BUF_SIZE] = "";
    char header[] = "UDP Client 1 ";
    int message_len, total_len, header_len = strlen(header);
    int str_len, recv_cnt;
    socklen_t recv_len;
    char recv_buf[BUF_SIZE];
    int i;

    // establish client socket
    sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock == -1)
        throw_error("socket() error");
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    // fetch user input
    printf("Enter a message: ");
    fgets(message, BUF_SIZE, stdin);
    message_len = strlen(message) - 1; // exclude newline
    total_len = header_len + sizeof(int) + message_len;

    // format the request
    char str_message_len[5];
    sprintf(str_message_len, "%d ", message_len);
    strcat(formatted_message,header);
    strcat(formatted_message,str_message_len);
    strcat(formatted_message,message);
    printf("Client: request formatted:  %s\n",formatted_message);

    // send request to server
    sendto(sock, formatted_message, total_len, 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    printf("Client: sent message to server...\n");

    // recieve server response
    recv_len = sizeof(serv_addr);
    recv_cnt = 0;
    while (recv_cnt < total_len) {
        str_len = recvfrom(sock, recv_buf, BUF_SIZE, 0, (struct sockaddr*)&serv_addr, &recv_len);
        if (str_len == -1)
            throw_error("Client: recvfrom() error");
        recv_cnt += str_len;
        printf("Client: received packet %d bytes\n", str_len);
    }
    
    recv_buf[recv_cnt] = '\0';
    printf("Client: received server response: %s\n",recv_buf);

    close(sock);
    printf("Client: connection closed\n");
    return 0;
}

/* Prints error message to console and exits
 */
void throw_error(char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
