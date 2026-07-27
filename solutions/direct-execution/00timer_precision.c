#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define ITERATIONS 1000000L

static long elapsed_us(
    const struct timeval *start,
    const struct timeval *end
) {
    return (end->tv_sec - start->tv_sec) * 1000000L
           + (end->tv_usec - start->tv_usec);
}

int main(void) {
    long zero_count = 0;
    long nonzero_count = 0;
    long minimum_nonzero = -1;
    long maximum = 0;

    for (long i = 0; i < ITERATIONS; i++) {
        struct timeval start;
        struct timeval end;

        if (gettimeofday(&start, NULL) == -1) {
            perror("gettimeofday");
            return EXIT_FAILURE;
        }

        if (gettimeofday(&end, NULL) == -1) {
            perror("gettimeofday");
            return EXIT_FAILURE;
        }

        long delta = elapsed_us(&start, &end);

        if (delta == 0) {
            zero_count++;
        } else {
            nonzero_count++;

            if (minimum_nonzero == -1 || delta < minimum_nonzero) {
                minimum_nonzero = delta;
            }

            if (delta > maximum) {
                maximum = delta;
            }
        }
    }

    printf("Iterations: %ld\n", ITERATIONS);
    printf("Zero differences: %ld\n", zero_count);
    printf("Non-zero differences: %ld\n", nonzero_count);
    printf("Minimum non-zero difference: %ld us\n", minimum_nonzero);
    printf("Maximum difference: %ld us\n", maximum);

    return EXIT_SUCCESS;
}