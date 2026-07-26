#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

int main(void)
{
    /*
     * 在 fork() 之前打开文件。
     *
     * O_WRONLY：只写
     * O_CREAT：文件不存在则创建
     * O_TRUNC：文件存在则清空
     *
     * 0644：文件所有者可读写，其他人只读
     */
    int fd = open("data.input",
                  O_WRONLY | O_CREAT | O_TRUNC,
                  0644);

    if (fd == -1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    printf("Before fork: pid=%d, fd=%d\n",
           (int)getpid(), fd);

    pid_t rc = fork();

    if (rc == -1)
    {
        perror("fork");
        close(fd);
        exit(EXIT_FAILURE);
    }

    if (rc == 0)
    {
        /* 子进程 */
        for (int i = 0; i < 10; i++)
        {
            char buffer[128];

            int length = snprintf(
                buffer,
                sizeof(buffer),
                "child: pid=%d, iteration=%d\n",
                (int)getpid(),
                i
            );

            if (write(fd, buffer, length) == -1)
            {
                perror("child write");
                close(fd);
                exit(EXIT_FAILURE);
            }

            /*
             * 让父子进程更容易交替执行。
             * usleep() 不是题目核心，只是方便观察结果。
             */
            usleep(10000);
        }

        close(fd);
        exit(EXIT_SUCCESS);
    }
    else
    {
        /* 父进程 */
        for (int i = 0; i < 10; i++)
        {
            char buffer[128];

            int length = snprintf(
                buffer,
                sizeof(buffer),
                "parent: pid=%d, iteration=%d\n",
                (int)getpid(),
                i
            );

            if (write(fd, buffer, length) == -1)
            {
                perror("parent write");
                close(fd);
                exit(EXIT_FAILURE);
            }

            usleep(10000);
        }

        wait(NULL);
        close(fd);
    }


    return 0;
}
