#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
    int pipefd[2];

    // pipefd[0]：管道读端
    // pipefd[1]：管道写端
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // 创建第一个子进程：负责执行 ls
    pid_t child1 = fork();

    if (child1 < 0) {
        perror("fork child1");
        exit(EXIT_FAILURE);
    }

    if (child1 == 0) {
        /*
         * 第一个子进程：
         * 把标准输出 STDOUT_FILENO 指向管道写端。
         */

        if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
            perror("dup2 child1");
            exit(EXIT_FAILURE);
        }

        // 已经通过 dup2 复制完成，原始管道描述符不再需要
        close(pipefd[0]);
        close(pipefd[1]);

        execlp("ls", "ls", NULL);

        // 只有 exec 失败才会运行到这里
        perror("execlp ls");
        exit(EXIT_FAILURE);
    }

    // 创建第二个子进程：负责执行 wc -l
    pid_t child2 = fork();

    if (child2 < 0) {
        perror("fork child2");
        exit(EXIT_FAILURE);
    }

    if (child2 == 0) {
        /*
         * 第二个子进程：
         * 把标准输入 STDIN_FILENO 指向管道读端。
         */

        if (dup2(pipefd[0], STDIN_FILENO) == -1) {
            perror("dup2 child2");
            exit(EXIT_FAILURE);
        }

        close(pipefd[0]);
        close(pipefd[1]);

        execlp("wc", "wc", "-l", NULL);

        perror("execlp wc");
        exit(EXIT_FAILURE);
    }

    /*
     * 父进程不参与管道通信，因此必须关闭管道的两个端点。
     */
    close(pipefd[0]);
    close(pipefd[1]);

    // 回收两个子进程
    waitpid(child1, NULL, 0);
    waitpid(child2, NULL, 0);

    return 0;
}