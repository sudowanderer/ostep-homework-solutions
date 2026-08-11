#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>

#define NUMCPUS 8
#define LOOPS 1000000

typedef struct __counter_t {
    int global;                         // global count
    pthread_mutex_t glock;              // global lock

    int local[NUMCPUS];                 // per-CPU count
    pthread_mutex_t llock[NUMCPUS];     // per-CPU locks

    int threshold;                      // update frequency
} counter_t;

counter_t counter;


// ------------------------------
// OSTEP Figure 29.4
// ------------------------------

void init(counter_t *c, int threshold) {
    c->threshold = threshold;
    c->global = 0;

    pthread_mutex_init(&c->glock, NULL);

    for (int i = 0; i < NUMCPUS; i++) {
        c->local[i] = 0;
        pthread_mutex_init(&c->llock[i], NULL);
    }
}


void update(counter_t *c, int threadID, int amt) {
    int cpu = threadID % NUMCPUS;

    pthread_mutex_lock(&c->llock[cpu]);

    c->local[cpu] += amt;

    if (c->local[cpu] >= c->threshold) {

        pthread_mutex_lock(&c->glock);

        c->global += c->local[cpu];

        pthread_mutex_unlock(&c->glock);

        c->local[cpu] = 0;
    }

    pthread_mutex_unlock(&c->llock[cpu]);
}


int get(counter_t *c) {
    pthread_mutex_lock(&c->glock);

    int val = c->global;

    pthread_mutex_unlock(&c->glock);

    return val;
}


// ------------------------------
// Benchmark
// ------------------------------

long get_time_us(void) {
    struct timeval tv;

    gettimeofday(&tv, NULL);

    return tv.tv_sec * 1000000L + tv.tv_usec;
}


void *worker(void *arg) {
    int threadID = *(int *)arg;

    for (long i = 0; i < LOOPS; i++) {
        update(&counter, threadID, 1);
    }

    return NULL;
}


int main(int argc, char *argv[]) {

    if (argc != 3) {
        fprintf(stderr,
                "Usage: %s <threads> <threshold>\n",
                argv[0]);

        return 1;
    }

    int num_threads = atoi(argv[1]);
    int threshold = atoi(argv[2]);

    init(&counter, threshold);

    pthread_t *threads =
        malloc(sizeof(pthread_t) * num_threads);

    int *thread_ids =
        malloc(sizeof(int) * num_threads);


    printf("CPUs available: %ld\n",
           sysconf(_SC_NPROCESSORS_ONLN));

    printf("NUMCPUS: %d\n", NUMCPUS);
    printf("Threads: %d\n", num_threads);
    printf("Threshold: %d\n", threshold);
    printf("Loops per thread: %d\n", LOOPS);


    long start = get_time_us();


    // 创建线程
    for (int i = 0; i < num_threads; i++) {

        thread_ids[i] = i;

        pthread_create(
            &threads[i],
            NULL,
            worker,
            &thread_ids[i]
        );
    }


    // 等待所有线程
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }


    long end = get_time_us();


    long total_operations =
        (long)num_threads * LOOPS;


    printf("Approximate global counter: %d\n",
           get(&counter));

    // 为了验证最终结果，把 local 也算进去
    long exact_count = counter.global;

    for (int i = 0; i < NUMCPUS; i++) {
        exact_count += counter.local[i];
    }

    printf("Exact final counter: %ld\n",
           exact_count);

    printf("Expected counter: %ld\n",
           total_operations);

    printf("Total time: %ld us\n",
           end - start);

    printf("Average: %.3f ns/increment\n",
           (end - start) * 1000.0
           / total_operations);


    // cleanup
    pthread_mutex_destroy(&counter.glock);

    for (int i = 0; i < NUMCPUS; i++) {
        pthread_mutex_destroy(&counter.llock[i]);
    }

    free(threads);
    free(thread_ids);

    return 0;
}