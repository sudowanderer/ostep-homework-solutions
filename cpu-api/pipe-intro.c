#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
    int pipefd[2];

    /*
     * pipefd[0]：管道读端
     * pipefd[1]：管道写端
     */
    if (pipe(pipefd) == -1)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t rc = fork();

    if (rc == -1)
    {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        exit(EXIT_FAILURE);
    }

    if (rc == 0)
    {
        /* 子进程不需要读取管道 */
        close(pipefd[0]);

        printf("hello\n");

        /*
         * 向管道写入一个字节，通知父进程：
         * “hello 已经打印完成”
         */
        char signal = 'x';

        if (write(pipefd[1], &signal, 1) == -1)
        {
            perror("child write");
            close(pipefd[1]);
            exit(EXIT_FAILURE);
        }

        close(pipefd[1]);
        exit(EXIT_SUCCESS);
    }
    else
    {
        /* 父进程不需要写入管道 */
        close(pipefd[1]);

        char signal;

        /*
         * 如果子进程还没有写入数据，
         * 父进程会阻塞在这里。
         */
        if (read(pipefd[0], &signal, 1) == -1)
        {
            perror("parent read");
            close(pipefd[0]);
            exit(EXIT_FAILURE);
        }

        printf("goodbye\n");

        close(pipefd[0]);
    }

    return 0;
}