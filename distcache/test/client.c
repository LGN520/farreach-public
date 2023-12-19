#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SERVER_IP "192.168.1.104"
#define SERVER_PORT 10086
void print_hex(char* buffer, int len) {
    int i;
    for (i = 1; i <= strlen(buffer); i++) {
        printf("0x%02X ", buffer[i - 1]);
        if (i % 16 == 0) {
            printf(" ");
        }
    }
}

int main() {
    int socket_fd;
    struct sockaddr_in server_addr;

    // 创建UDP套接字
    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // 设置服务器地址
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &(server_addr.sin_addr));

    // 发送数字到服务器
    int sum = 0;
    for (int i = 0; i < 2520; i++) {
        int number_to_send = i;
        sendto(socket_fd, &number_to_send, sizeof(number_to_send), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));

        // printf("Sent number: %d ", number_to_send);

        // 接收服务器返回的数字
        char ackbuf[1024];
        recvfrom(socket_fd, ackbuf, 1024, 0, NULL, NULL);
        if(((int*)ackbuf)[1]){
        printf("ackbuf %d \n", strlen((char*)&number_to_send));
        // print_hex(ackbuf, strlen(ackbuf));
        printf("Received frequency: %d %d \n", ((int*)ackbuf)[0], ((int*)ackbuf)[1]);}
        sum += ((int*)ackbuf)[1];
    }
    printf("sum: %d\n", sum);
    // 关闭套接字
    close(socket_fd);

    return 0;
}