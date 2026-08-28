#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>

#define PORT 8080
#define BUFFER_SIZE 4096

int main(void) {

    // =========================================================
    // 1. 创建 TCP Server（样板代码）
    // =========================================================

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(server_fd,
               SOL_SOCKET,
               SO_REUSEADDR,
               &opt,
               sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(server_fd,
             (struct sockaddr *)&addr,
             sizeof(addr)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("File server listening on port %d\n", PORT);


    // =========================================================
    // 2. select() 初始化
    // =========================================================

    fd_set master_set;
    fd_set read_set;

    FD_ZERO(&master_set);

    // 监听 server socket
    FD_SET(server_fd, &master_set);

    int max_fd = server_fd;


    // =========================================================
    // 3. Event Loop
    // =========================================================

    while (1) {

        // select() 会修改 fd_set，所以复制
        read_set = master_set;

        printf("waiting for events...\n");

        int ready = select(
            max_fd + 1,
            &read_set,
            NULL,
            NULL,
            NULL
        );

        if (ready < 0) {
            perror("select");
            exit(EXIT_FAILURE);
        }


        // =====================================================
        // 4. 找出哪些 fd ready
        // =====================================================

        for (int fd = 0; fd <= max_fd; fd++) {

            if (!FD_ISSET(fd, &read_set)) {
                continue;
            }


            // =================================================
            // 5. server_fd ready：
            //    有新的 TCP connection
            // =================================================

            if (fd == server_fd) {

                int client_fd =
                    accept(server_fd, NULL, NULL);

                if (client_fd < 0) {
                    perror("accept");
                    continue;
                }

                printf("new client: fd=%d\n", client_fd);

                // 开始监听这个 client
                FD_SET(client_fd, &master_set);

                if (client_fd > max_fd) {
                    max_fd = client_fd;
                }
            }


            // =================================================
            // 6. client_fd ready：
            //    client 发来了文件名
            // =================================================

            else {

                char filename[256];

                ssize_t n =
                    read(fd,
                         filename,
                         sizeof(filename) - 1);

                // client 断开
                if (n <= 0) {

                    printf("client closed: fd=%d\n", fd);

                    close(fd);
                    FD_CLR(fd, &master_set);

                    continue;
                }

                filename[n] = '\0';

                // nc 输入通常带 \n
                // 去掉 \r 和 \n
                filename[strcspn(filename, "\r\n")] = '\0';

                printf(
                    "client fd=%d requests file: %s\n",
                    fd,
                    filename
                );


                // =================================================
                // 7. 打开文件
                // =================================================

                int file_fd =
                    open(filename, O_RDONLY);

                if (file_fd < 0) {

                    char *msg = "file not found\n";

                    write(
                        fd,
                        msg,
                        strlen(msg)
                    );

                    close(fd);
                    FD_CLR(fd, &master_set);

                    continue;
                }


                printf("fd=%d: simulate slow file I/O...\n", fd);

                sleep(10);   // 模拟 read(file_fd) 阻塞 10 秒

                printf("fd=%d: file I/O ready\n", fd);


                // =================================================
                // 8. 读取文件，并发送给 client
                // =================================================

                char buffer[BUFFER_SIZE];

                while (1) {

                    ssize_t bytes_read =
                        read(
                            file_fd,
                            buffer,
                            sizeof(buffer)
                        );

                    if (bytes_read < 0) {
                        perror("read file");
                        break;
                    }

                    // EOF
                    if (bytes_read == 0) {
                        break;
                    }

                    /*
                     * 把刚刚从文件读取的数据
                     * 写入 TCP connection
                     */
                    ssize_t bytes_written =
                        write(
                            fd,
                            buffer,
                            bytes_read
                        );

                    if (bytes_written < 0) {
                        perror("write client");
                        break;
                    }
                }


                // =================================================
                // 9. 请求处理完成
                // =================================================

                close(file_fd);

                close(fd);
                FD_CLR(fd, &master_set);

                printf(
                    "request finished: fd=%d\n",
                    fd
                );
            }
        }
    }


    close(server_fd);

    return 0;
}