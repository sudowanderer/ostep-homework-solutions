#include <stdio.h>
#include <sys/time.h>
#include <stdint.h>

static int64_t diff_us(struct timeval start, struct timeval end)
{
    return (end.tv_sec - start.tv_sec) * 1000000LL
         + (end.tv_usec - start.tv_usec);
}

int main(void)
{
    struct timeval start, end;

    // 1. 测量连续两次 gettimeofday() 的时间差
    gettimeofday(&start, NULL);
    gettimeofday(&end, NULL);

    printf("Two consecutive gettimeofday(): %lld us\n",
           (long long)diff_us(start, end));


    // 2. 测量大量调用，估算一次 gettimeofday() 的平均成本
    const int N = 1000000;

    gettimeofday(&start, NULL);

    for (int i = 0; i < N; i++) {
        struct timeval tmp;
        gettimeofday(&tmp, NULL);
    }

    gettimeofday(&end, NULL);

    int64_t total_us = diff_us(start, end);

    printf("\n%d calls:\n", N);
    printf("Total time: %lld us\n", (long long)total_us);
    printf("Average: %.3f us/call\n",
           (double)total_us / N);


    // 3. 找到 gettimeofday() 能观察到的最小非零时间间隔
    int64_t min_diff = INT64_MAX;

    for (int i = 0; i < N; i++) {
        struct timeval a, b;

        gettimeofday(&a, NULL);

        do {
            gettimeofday(&b, NULL);
        } while (diff_us(a, b) == 0);

        int64_t d = diff_us(a, b);

        if (d < min_diff)
            min_diff = d;
    }

    printf("\nMinimum observable non-zero interval: %lld us\n",
           (long long)min_diff);

    return 0;
}