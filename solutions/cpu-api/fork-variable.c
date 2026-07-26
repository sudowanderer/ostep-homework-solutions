#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
    int x = 100;

    printf("before fork: pid=%d, x=%d, address=%p\n", (int)getpid(), x, (void*)&x);

    int rc = fork();
    if (rc < 0)
    {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }
    else if (rc == 0)
    {
        // child process
        printf("child before change: pid=%d, x=%d, address=%p\n", (int)getpid(), x, (void*)&x);

        x = 200;

        printf("child after change: pid=%d, x=%d, address=%p\n", (int)getpid(), x, (void*)&x);
    }
    else
    {
        // parent process
        printf("parent before change: pid=%d, x=%d, address=%p\n", (int)getpid(), x, (void*)&x);

        x = 300;

        printf("parent after change: pid=%d, x=%d, address=%p\n", (int)getpid(), x, (void*)&x);

        wait(NULL);
    }
    
    printf("hello world\n");

    return 0;
}
