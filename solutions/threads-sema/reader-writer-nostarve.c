#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "common_threads.h"

//
// Your code goes in the structure and functions below
//

typedef struct __rwlock_t
{
    int readers;

    // 保护 readers 计数器

    sem_t readers_lock;

    // 控制共享资源：

    // 要么一群 readers 使用，要么一个 writer 使用

    sem_t write_lock;

    // 公平性入口：

    // writer 到达后，阻止新的 reader 继续插队

    sem_t turnstile;
} rwlock_t;

void rwlock_init(rwlock_t* rw)
{
    rw->readers = 0;

    sem_init(&rw->readers_lock, 0, 1);

    sem_init(&rw->write_lock, 0, 1);

    sem_init(&rw->turnstile, 0, 1);
}

void rwlock_acquire_readlock(rwlock_t* rw)
{
    /*

     * Reader 必须先经过 turnstile。

     *

     * 正常情况下：

     *     wait -> post

     * 很快通过。

     *

     * 如果已经有 Writer 拿着 turnstile 等待，

     * 新来的 Reader 就会被挡在这里，

     * 从而防止 Reader 无限插队。

     */

    sem_wait(&rw->turnstile);

    sem_post(&rw->turnstile);

    // 修改 readers 前先加锁

    sem_wait(&rw->readers_lock);

    rw->readers++;

    /*

     * 第一个 Reader 代表整个 Reader 群体

     * 获取 write_lock，阻止 Writer 进入。

     */

    if (rw->readers == 1)
    {
        sem_wait(&rw->write_lock);
    }

    sem_post(&rw->readers_lock);
}

void rwlock_release_readlock(rwlock_t* rw)
{
    sem_wait(&rw->readers_lock);

    rw->readers--;

    /*

     * 最后一个 Reader 离开：

     * Reader 群体释放 write_lock，

     * Writer 现在可以进入。

     */

    if (rw->readers == 0)
    {
        sem_post(&rw->write_lock);
    }

    sem_post(&rw->readers_lock);
}

void rwlock_acquire_writelock(rwlock_t* rw)
{
    /*

     * Writer 首先关闭入口。

     *

     * 从这一刻开始，新来的 Reader

     * 无法再继续插队。

     */

    sem_wait(&rw->turnstile);

    /*

     * 然后等待当前正在读取的 Reader

     * 全部离开。

     */

    sem_wait(&rw->write_lock);
}

void rwlock_release_writelock(rwlock_t* rw)
{
    // 先释放真正的共享资源

    sem_post(&rw->write_lock);

    // 再重新打开入口

    sem_post(&rw->turnstile);
}

//
// Don't change the code below (just use it!)
//

int loops;
int value = 0;

rwlock_t lock;

void* reader(void* arg)
{
    int i;
    for (i = 0; i < loops; i++)
    {
        rwlock_acquire_readlock(&lock);
        printf("read %d\n", value);
        rwlock_release_readlock(&lock);
    }
    return NULL;
}

void* writer(void* arg)
{
    int i;
    for (i = 0; i < loops; i++)
    {
        rwlock_acquire_writelock(&lock);
        value++;
        printf("write %d\n", value);
        rwlock_release_writelock(&lock);
    }
    return NULL;
}

int main(int argc, char* argv[])
{
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
