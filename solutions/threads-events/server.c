#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main(void) {
    int server_fd;
    struct sockaddr_in server_addr;

    // 1. 创建 TCP socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // 方便程序重启后立即重新绑定端口
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
                   &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 2. 设置服务器地址
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // 3. 把 socket 绑定到 8080 端口
    if (bind(server_fd,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0) {
        perror("bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 4. 开始监听
    if (listen(server_fd, 10) < 0) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    // 5. 不断接受客户端连接
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        printf("Waiting for connection...\n");

        int client_fd = accept(
            server_fd,
            (struct sockaddr *)&client_addr,
            &client_len
        );

        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        printf("Client connected: %s:%d\n",
               inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port));

        // 6. 等待客户端发送一个请求
        char buffer[BUFFER_SIZE];

        ssize_t n = read(client_fd, buffer, sizeof(buffer) - 1);

        if (n < 0) {
            perror("read");
            close(client_fd);
            continue;
        }

        if (n == 0) {
            printf("Client closed connection\n");
            close(client_fd);
            continue;
        }

        buffer[n] = '\0';

        printf("Request: %s\n", buffer);

        // 7. 获取当前时间
        time_t now = time(NULL);
        char *time_string = ctime(&now);

        // 8. 返回给客户端
        if (write(client_fd, time_string, strlen(time_string)) < 0) {
            perror("write");
        }

        // 9. 一个请求处理完成，直接关闭连接
        close(client_fd);

        printf("Request finished.\n\n");
    }

    close(server_fd);

    return 0;
}