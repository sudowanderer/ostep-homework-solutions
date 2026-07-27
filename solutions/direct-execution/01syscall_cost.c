#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#define ITERATIONS 10000000L

static double elapsed_us(
    const struct timeval *start,
    const struct timeval *end
) {
    return (end->tv_sec - start->tv_sec) * 1000000.0
           + (end->tv_usec - start->tv_usec);
}

int main(void) {
    struct timeval start;
    struct timeval end;

    /*
     * 打开 /dev/null，避免真实磁盘 I/O。
     *
     * 这里使用 read(fd, ..., 0)：
     * 会进入内核，但不会真正读取数据。
     */
    int fd = open("/dev/null", O_RDONLY);
    if (fd == -1) {
        perror("open");
        return EXIT_FAILURE;
    }

    char buffer;

    /*
     * 预热（warm-up）。
     * 减少第一次调用、缓存状态等因素对测量结果的影响。
     */
    for (long i = 0; i < 10000; i++) {
        if (read(fd, &buffer, 0) == -1) {
            perror("read");
            close(fd);
            return EXIT_FAILURE;
        }
    }

    if (gettimeofday(&start, NULL) == -1) {
        perror("gettimeofday");
        close(fd);
        return EXIT_FAILURE;
    }

    for (long i = 0; i < ITERATIONS; i++) {
        ssize_t result = read(fd, &buffer, 0);

        if (result == -1) {
            perror("read");
            close(fd);
            return EXIT_FAILURE;
        }
    }

    if (gettimeofday(&end, NULL) == -1) {
        perror("gettimeofday");
        close(fd);
        return EXIT_FAILURE;
    }

    close(fd);

    double total_us = elapsed_us(&start, &end);
    double average_us = total_us / ITERATIONS;
    double average_ns = average_us * 1000.0;

    printf("Iterations: %ld\n", ITERATIONS);
    printf("Total time: %.3f us\n", total_us);
    printf("Average syscall time: %.3f ns\n", average_ns);

    return EXIT_SUCCESS;
}