#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

static uint64_t now_ns(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <pages> <trials>\n", argv[0]);
        return 1;
    }

    long num_pages = atol(argv[1]);
    long trials = atol(argv[2]);

    long page_size = sysconf(_SC_PAGESIZE);

    // 一个 page 能放多少个 int
    long jump = page_size / sizeof(int);

    // 总共需要多少个 int
    size_t num_ints = (size_t)num_pages * jump;

    int *a = calloc(num_ints, sizeof(int));

    if (a == NULL) {
        perror("calloc");
        return 1;
    }

    /*
     * Warm-up:
     * 提前 touch 每一个 page，
     * 避免正式计时的时候混入首次访问 page 的成本。
     */
    for (long page = 0; page < num_pages; page++) {
        a[page * jump] = 1;
    }

    uint64_t start = now_ns();

    for (long t = 0; t < trials; t++) {
        for (size_t i = 0; i < num_ints; i += jump) {
            a[i] += 1;
        }
    }

    uint64_t end = now_ns();

    uint64_t total_accesses =
        (uint64_t)num_pages * (uint64_t)trials;

    double ns_per_access =
        (double)(end - start) / total_accesses;

    /*
     * 使用结果，降低编译器把整个循环优化掉的可能性。
     */
    long long checksum = 0;

    for (long page = 0; page < num_pages; page++) {
        checksum += a[page * jump];
    }

    printf("Page size:       %ld bytes\n", page_size);
    printf("Pages:           %ld\n", num_pages);
    printf("Trials:          %ld\n", trials);
    printf("Total accesses:  %llu\n",
           (unsigned long long)total_accesses);
    printf("Time:            %.3f ns/access\n",
           ns_per_access);
    printf("Checksum:        %lld\n", checksum);

    free(a);

    return 0;
}