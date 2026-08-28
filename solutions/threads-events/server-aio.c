// server-aio.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <aio.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>

#define PORT 8080
#define BUFFER_SIZE 4096
#define MAX_REQUESTS 1024

/*
 * 每一个异步文件读取请求，都必须保存自己的状态。
 */
typedef struct {
    int active;

    int client_fd;
    int file_fd;

    char buffer[BUFFER_SIZE];

    struct aiocb aio;
} aio_request;

aio_request requests[MAX_REQUESTS];


int main(void) {

    // =========================================================
    // TCP Server 样板代码
    // =========================================================

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    int opt = 1;

    setsockopt(
        server_fd,
        SOL_SOCKET,
        SO_REUSEADDR,
        &opt,
        sizeof(opt)
    );

    struct sockaddr_in addr;

    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(
            server_fd,
            (struct sockaddr *)&addr,
            sizeof(addr)
        ) < 0) {

        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("AIO file server listening on port %d\n", PORT);


    // =========================================================
    // select()
    // =========================================================

    fd_set master_set;
    fd_set read_set;

    FD_ZERO(&master_set);
    FD_SET(server_fd, &master_set);

    int max_fd = server_fd;


    // =========================================================
    // Event Loop
    // =========================================================

    while (1) {

        /*
         * 这里不能永远阻塞。
         *
         * 因为除了 socket event，
         * 我们还需要定期检查 AIO 是否完成。
         */
        read_set = master_set;

        struct timeval timeout;

        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;   // 100 ms


        int ready = select(
            max_fd + 1,
            &read_set,
            NULL,
            NULL,
            &timeout
        );

        if (ready < 0) {
            perror("select");
            exit(EXIT_FAILURE);
        }


        // =====================================================
        // 1. 处理 socket events
        // =====================================================

        for (int fd = 0; fd <= max_fd; fd++) {

            if (!FD_ISSET(fd, &read_set)) {
                continue;
            }


            // -------------------------------------------------
            // 新 connection
            // -------------------------------------------------

            if (fd == server_fd) {

                int client_fd =
                    accept(server_fd, NULL, NULL);

                if (client_fd < 0) {
                    perror("accept");
                    continue;
                }

                printf(
                    "new client: fd=%d\n",
                    client_fd
                );

                FD_SET(client_fd, &master_set);

                if (client_fd > max_fd) {
                    max_fd = client_fd;
                }
            }


            // -------------------------------------------------
            // client 发来文件名
            // -------------------------------------------------

            else {

                char filename[256];

                ssize_t n =
                    read(
                        fd,
                        filename,
                        sizeof(filename) - 1
                    );

                if (n <= 0) {

                    close(fd);
                    FD_CLR(fd, &master_set);

                    continue;
                }

                filename[n] = '\0';

                filename[
                    strcspn(filename, "\r\n")
                ] = '\0';

                printf(
                    "client fd=%d requests: %s\n",
                    fd,
                    filename
                );


                // -------------------------------------------------
                // 打开文件
                // -------------------------------------------------

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


                // -------------------------------------------------
                // 找一个空闲的 request slot
                // -------------------------------------------------

                int slot = -1;

                for (int i = 0; i < MAX_REQUESTS; i++) {

                    if (!requests[i].active) {

                        slot = i;

                        break;
                    }
                }

                if (slot < 0) {

                    char *msg =
                        "server busy\n";

                    write(
                        fd,
                        msg,
                        strlen(msg)
                    );

                    close(file_fd);
                    close(fd);

                    FD_CLR(fd, &master_set);

                    continue;
                }


                aio_request *req =
                    &requests[slot];

                memset(req, 0, sizeof(*req));

                req->active = 1;

                req->client_fd = fd;
                req->file_fd = file_fd;


                // -------------------------------------------------
                // 构造 aiocb
                // -------------------------------------------------

                req->aio.aio_fildes =
                    file_fd;

                req->aio.aio_buf =
                    req->buffer;

                req->aio.aio_nbytes =
                    BUFFER_SIZE;

                req->aio.aio_offset =
                    0;


                // -------------------------------------------------
                // ★ 真正关键：提交异步读取
                // -------------------------------------------------

                if (aio_read(&req->aio) < 0) {

                    perror("aio_read");

                    close(file_fd);
                    close(fd);

                    FD_CLR(fd, &master_set);

                    req->active = 0;

                    continue;
                }


                printf(
                    "AIO submitted: client=%d file_fd=%d\n",
                    fd,
                    file_fd
                );


                /*
                 * 非常重要：
                 *
                 * 不在这里等待！
                 *
                 * client fd 暂时从 select 中移除。
                 * 等文件读取完成以后再响应。
                 */

                FD_CLR(fd, &master_set);
            }
        }


        // =====================================================
        // 2. 检查异步文件 I/O 是否完成
        // =====================================================

        for (int i = 0; i < MAX_REQUESTS; i++) {

            aio_request *req =
                &requests[i];

            if (!req->active) {
                continue;
            }


            int status =
                aio_error(&req->aio);


            /*
             * 还没完成。
             *
             * Event Loop 不等待它，
             * 继续处理其他事情。
             */
            if (status == EINPROGRESS) {
                continue;
            }


            // -------------------------------------------------
            // AIO 出错
            // -------------------------------------------------

            if (status != 0) {

                fprintf(
                    stderr,
                    "AIO error: %s\n",
                    strerror(status)
                );

                close(req->file_fd);
                close(req->client_fd);

                req->active = 0;

                continue;
            }


            // -------------------------------------------------
            // ★ AIO 完成
            // -------------------------------------------------

            ssize_t bytes_read =
                aio_return(&req->aio);


            printf(
                "AIO completed: client=%d bytes=%zd\n",
                req->client_fd,
                bytes_read
            );


            if (bytes_read > 0) {

                write(
                    req->client_fd,
                    req->buffer,
                    bytes_read
                );
            }


            // -------------------------------------------------
            // 请求完成
            // -------------------------------------------------

            close(req->file_fd);
            close(req->client_fd);

            req->active = 0;
        }
    }


    close(server_fd);

    return 0;
}