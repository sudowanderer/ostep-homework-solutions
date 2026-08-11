#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>

typedef struct {
    long value;
    pthread_mutex_t lock;
} counter_t;

counter_t counter;

// 每个线程执行多少次 ++
long loops_per_thread = 1000000;

void *worker(void *arg) {
    for (long i = 0; i < loops_per_thread; i++) {
        pthread_mutex_lock(&counter.lock);

        counter.value++;

        pthread_mutex_unlock(&counter.lock);
    }

    return NULL;
}

long get_time_us(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return tv.tv_sec * 1000000L + tv.tv_usec;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <threads>\n", argv[0]);
        return 1;
    }

    int num_threads = atoi(argv[1]);

    printf("CPUs available: %ld\n", sysconf(_SC_NPROCESSORS_ONLN));
    printf("Threads: %d\n", num_threads);
    printf("Loops per thread: %ld\n", loops_per_thread);

    counter.value = 0;
    pthread_mutex_init(&counter.lock, NULL);

    pthread_t *threads =
        malloc(sizeof(pthread_t) * num_threads);

    long start = get_time_us();

    for (int i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, worker, NULL);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    long end = get_time_us();

    printf("Counter: %ld\n", counter.value);
    printf("Total time: %ld us\n", end - start);
    printf("Average: %.3f ns/increment\n",
           (end - start) * 1000.0 / counter.value);

    pthread_mutex_destroy(&counter.lock);
    free(threads);

    return 0;
}