#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "common_threads.h"

//
// Your code goes in the structure and functions below
//

typedef struct __rwlock_t {
	int readers;
	sem_t readerslock;
	sem_t writerslock;
} rwlock_t;

void rwlock_init(rwlock_t *rw) {
	rw->readers = 0;
	sem_init(&rw->readerslock, 0, 1);
	sem_init(&rw->writerslock, 0, 1);
}

void rwlock_acquire_readlock(rwlock_t *rw) {
	sem_wait(&rw->readerslock);

	rw->readers++;

	if (rw->readers == 1) {
		// 第一个 reader 代表所有 readers 阻止 writer 进入
		sem_wait(&rw->writerslock);
	}

	sem_post(&rw->readerslock);
}

void rwlock_release_readlock(rwlock_t *rw) {
	sem_wait(&rw->readerslock);

	rw->readers--;

	if (rw->readers == 0) {
		// 最后一个 reader 离开，允许 writer 进入
		sem_post(&rw->writerslock);
	}

	sem_post(&rw->readerslock);
}

void rwlock_acquire_writelock(rwlock_t *rw) {
	sem_wait(&rw->writerslock);
}

void rwlock_release_writelock(rwlock_t *rw) {
	sem_post(&rw->writerslock);
}

//
// Don't change the code below (just use it!)
// 

int loops;
int value = 0;

rwlock_t lock;

void *reader(void *arg) {
    int i;
    for (i = 0; i < loops; i++) {
	rwlock_acquire_readlock(&lock);
	printf("read %d\n", value);
	rwlock_release_readlock(&lock);
    }
    return NULL;
}

void *writer(void *arg) {
    int i;
    for (i = 0; i < loops; i++) {
	rwlock_acquire_writelock(&lock);
	value++;
	printf("write %d\n", value);
	rwlock_release_writelock(&lock);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    assert(argc == 4);
    int num_readers = atoi(argv[1]);
    int num_writers = atoi(argv[2]);
    loops = atoi(argv[3]);

    pthread_t pr[num_readers], pw[num_writers];

    rwlock_init(&lock);

    printf("begin\n");

    int i;
    for (i = 0; i < num_readers; i++)
	Pthread_create(&pr[i], NULL, reader, NULL);
    for (i = 0; i < num_writers; i++)
	Pthread_create(&pw[i], NULL, writer, NULL);

    for (i = 0; i < num_readers; i++)
	Pthread_join(pr[i], NULL);
    for (i = 0; i < num_writers; i++)
	Pthread_join(pw[i], NULL);

    printf("end: value %d\n", value);

    return 0;
}

