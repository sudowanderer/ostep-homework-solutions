// memory-user.c — OSTEP vm-intro: allocate and touch memory to observe RSS

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <memory_in_MB>\n", argv[0]);
        return 1;
    }

    long mb = atol(argv[1]);
    if (mb <= 0) {
        fprintf(stderr, "memory_in_MB must be a positive integer\n");
        return 1;
    }

    size_t size = (size_t)mb * 1024 * 1024;

    printf("pid: %d\n", getpid());
    printf("Allocating %ld MB...\n", mb);

    char *array = malloc(size);
    if (array == NULL) {
        perror("malloc");
        return 1;
    }

    printf("Memory allocated at %p\n", (void *)array);
    printf("Touching memory forever (Ctrl-C to stop)...\n");

    // Keep pages resident by writing one byte per page.
    while (1) {
        for (size_t i = 0; i < size; i += 4096) {
            array[i]++;
        }
        usleep(10000);
    }

    free(array);
    return 0;
}
