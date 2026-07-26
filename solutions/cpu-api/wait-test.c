#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

int main(void)
{
    printf("before fork: pid=%d\n", getpid());

    pid_t rc = fork();

    if (rc < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (rc == 0) {
        // child process
        printf("child: pid=%d, parent pid=%d\n",
               getpid(), getppid());

        printf("child: calling wait()...\n");

        int status;
        pid_t wait_rc = wait(&status);

        printf("child: wait() returned %d\n", wait_rc);

        if (wait_rc == -1) {
            printf("child: wait() failed: %s\n", strerror(errno));
        }

        printf("child: exiting\n");
        exit(42);
    } else {
        // parent process
        printf("parent: pid=%d, child pid=%d\n",
               getpid(), rc);

        printf("parent: waiting for child...\n");

        int status;
        pid_t wait_rc = wait(&status);

        printf("parent: wait() returned %d\n", wait_rc);

        if (wait_rc == -1) {
            perror("wait");
            exit(EXIT_FAILURE);
        }

        if (WIFEXITED(status)) {
            printf("parent: child exited normally, exit code=%d\n",
                   WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("parent: child was terminated by signal %d\n",
                   WTERMSIG(status));
        }

        printf("parent: exiting\n");
    }

    return 0;
}