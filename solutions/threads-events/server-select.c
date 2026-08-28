#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main(void) {
    // ===== 样板代码：创建 TCP Server =====

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
               &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    bind(server_fd,
         (struct sockaddr *)&addr,
         sizeof(addr));

    listen(server_fd, 10);

    printf("Server listening on port %d\n", PORT);


    // ===== 从这里开始才是第二题重点 =====

    fd_set master_set;
    fd_set read_set;

    FD_ZERO(&master_set);

    // server_fd 也需要被 select() 监听
    FD_SET(server_fd, &master_set);

    int max_fd = server_fd;

    while (1) {

        /*
         * select() 会修改 fd_set，
         * 所以每轮必须复制一份。
         */
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

        /*
         * 检查所有 fd，看看是谁 ready 了。
         */
        for (int fd = 0; fd <= max_fd; fd++) {

            if (!FD_ISSET(fd, &read_set))
                continue;

            /*
             * 情况 1：
             * server_fd ready
             *
             * 说明有新的 TCP connection。
             */
            if (fd == server_fd) {

                int client_fd =
                    accept(server_fd, NULL, NULL);

                if (client_fd < 0) {
                    perror("accept");
                    continue;
                }

                printf("new client: fd=%d\n", client_fd);

                // 从下一轮开始监听这个 client
                FD_SET(client_fd, &master_set);

                if (client_fd > max_fd)
                    max_fd = client_fd;
            }

            /*
             * 情况 2：
             * client_fd ready
             *
             * 说明这个 connection 上有数据可以读。
             */
            else {

                char buffer[BUFFER_SIZE];

                ssize_t n =
                    read(fd, buffer, sizeof(buffer) - 1);

                /*
                 * client 关闭连接
                 */
                if (n <= 0) {

                    printf("client closed: fd=%d\n", fd);

                    close(fd);

                    // 不再监听这个 fd
                    FD_CLR(fd, &master_set);
                }

                /*
                 * 收到 request
                 */
                else {

                    buffer[n] = '\0';

                    printf("request from fd=%d: %s\n",
                           fd, buffer);

                    time_t now = time(NULL);
                    char *response = ctime(&now);

                    write(fd,
                          response,
                          strlen(response));

                    /*
                     * 和第一题一样：
                     * 一个 request 后关闭 connection。
                     */
                    close(fd);
                    FD_CLR(fd, &master_set);
                }
            }
        }
    }
}