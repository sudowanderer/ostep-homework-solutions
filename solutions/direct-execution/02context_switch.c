#define _GNU_SOURCE

#include <errno.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

#define WARMUP_ITERATIONS 10000L
#define MEASURE_ITERATIONS 100000L
#define TARGET_CPU 0

static double elapsed_us(
    const struct timeval *start,
    const struct timeval *end
) {
    return (end->tv_sec - start->tv_sec) * 1000000.0
           + (end->tv_usec - start->tv_usec);
}

static void bind_to_cpu(int cpu_number) {
    cpu_set_t cpu_set;

    CPU_ZERO(&cpu_set);
    CPU_SET(cpu_number, &cpu_set);

    if (sched_setaffinity(0, sizeof(cpu_set), &cpu_set) == -1) {
        perror("sched_setaffinity");
        exit(EXIT_FAILURE);
    }
}

static void close_or_die(int fd) {
    if (close(fd) == -1) {
        perror("close");
        exit(EXIT_FAILURE);
    }
}

static void read_one_byte(int fd, char *value) {
    ssize_t result;

    do {
        result = read(fd, value, 1);
    } while (result == -1 && errno == EINTR);

    if (result == -1) {
        perror("read");
        exit(EXIT_FAILURE);
    }

    if (result == 0) {
        fprintf(stderr, "Unexpected end of pipe\n");
        exit(EXIT_FAILURE);
    }
}

static void write_one_byte(int fd, char value) {
    ssize_t result;

    do {
        result = write(fd, &value, 1);
    } while (result == -1 && errno == EINTR);

    if (result == -1) {
        perror("write");
        exit(EXIT_FAILURE);
    }

    if (result != 1) {
        fprintf(stderr, "Incomplete pipe write\n");
        exit(EXIT_FAILURE);
    }
}

int main(void) {
    int parent_to_child[2];
    int child_to_parent[2];

    if (pipe(parent_to_child) == -1) {
        perror("pipe parent_to_child");
        return EXIT_FAILURE;
    }

    if (pipe(child_to_parent) == -1) {
        perror("pipe child_to_parent");
        return EXIT_FAILURE;
    }

    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        return EXIT_FAILURE;
    }

    long total_iterations =
        WARMUP_ITERATIONS + MEASURE_ITERATIONS;

    if (pid == 0) {
        bind_to_cpu(TARGET_CPU);

        close_or_die(parent_to_child[1]);
        close_or_die(child_to_parent[0]);

        char value;

        for (long i = 0; i < total_iterations; i++) {
            read_one_byte(parent_to_child[0], &value);
            write_one_byte(child_to_parent[1], value);
        }

        close_or_die(parent_to_child[0]);
        close_or_die(child_to_parent[1]);

        _exit(EXIT_SUCCESS);
    }

    bind_to_cpu(TARGET_CPU);

    close_or_die(parent_to_child[0]);
    close_or_die(child_to_parent[1]);

    char value = 'x';

    /*
     * 预热阶段，不计时。
     */
    for (long i = 0; i < WARMUP_ITERATIONS; i++) {
        write_one_byte(parent_to_child[1], value);
        read_one_byte(child_to_parent[0], &value);
    }

    struct timeval start;
    struct timeval end;

    if (gettimeofday(&start, NULL) == -1) {
        perror("gettimeofday");
        return EXIT_FAILURE;
    }

    for (long i = 0; i < MEASURE_ITERATIONS; i++) {
        write_one_byte(parent_to_child[1], value);
        read_one_byte(child_to_parent[0], &value);
    }

    if (gettimeofday(&end, NULL) == -1) {
        perror("gettimeofday");
        return EXIT_FAILURE;
    }

    close_or_die(parent_to_child[1]);
    close_or_die(child_to_parent[0]);

    int status;

    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid");
        return EXIT_FAILURE;
    }

    if (!WIFEXITED(status) || WEXITSTATUS(status) != EXIT_SUCCESS) {
        fprintf(stderr, "Child process exited abnormally\n");
        return EXIT_FAILURE;
    }

    double total_us = elapsed_us(&start, &end);

    /*
     * 一轮 Ping-Pong：
     *
     * parent -> child
     * child  -> parent
     *
     * 大致包含两次进程切换：
     *
     * parent -> child
     * child  -> parent
     */
    double round_trip_us =
        total_us / MEASURE_ITERATIONS;

    double estimated_switch_us =
        total_us / (MEASURE_ITERATIONS * 2.0);

    printf("Warm-up iterations: %ld\n", WARMUP_ITERATIONS);
    printf("Measured iterations: %ld\n", MEASURE_ITERATIONS);
    printf("Total measured time: %.3f us\n", total_us);
    printf("Average round-trip time: %.3f us\n", round_trip_us);
    printf("Approximate time per switch: %.3f us\n",
           estimated_switch_us);

    printf("\n");
    printf("Note: the per-switch value also contains pipe read/write,\n");
    printf("system-call and scheduler overhead. It is not a pure\n");
    printf("context-switch measurement.\n");

    return EXIT_SUCCESS;
}