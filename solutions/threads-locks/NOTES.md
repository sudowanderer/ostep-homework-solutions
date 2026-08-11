## [timer.c](timer.c)

这里做实验，测量每次gettimeofday调用的耗时。然后测量gettimeofday的精度，
也就是，连续两次gettimeofday调用，最小能察觉出时间变化的interval区间。
这个interval测量很重要，也是先把工具尺子的精度研究清楚。
```shell
Two consecutive gettimeofday(): 0 us

1000000 calls:
Total time: 8854 us
Average: 0.009 us/call

Minimum observable non-zero interval: 1 us
```

测量结果：精度那该是1微妙。也就是说，纳秒的调用时间变化，是测不出来的，需要大量循环调用计算平均值。

总体思想：
> 先搞清楚你的“尺子”怎么工作、精度是多少，再拿这把尺子去测锁、线程和并发数据结构。

在性能优化中，这很重要。

## [counter.c](counter.c)

每个线程执行1M次counter++，随着thread的增加，观测测量结果。发现，线程数，增加，每次increment的耗时反而增加了。
这就是系统性能没有横向伸缩提升的明显证明。

```text
1 thread   10.17 ns/increment
2 threads  12.99 ns/increment
4 threads  15.53 ns/increment
8 threads  18.30 ns/increment
16 threads 18.05 ns/increment
32 threads 18.40 ns/increment
```
为了更适合工程话，可以观测额外的指标：吞吐量
吞吐量（throughput），也就是每秒能完成多少次 increment：

```text
throughput = 总 increment 数 / Total time
```

我的实验结果：
```text
1 thread   ≈ 98 M increments/s
2 threads  ≈ 77 M increments/s
4 threads  ≈ 64 M increments/s
8 threads  ≈ 55 M increments/s
16 threads ≈ 55 M increments/s
32 threads ≈ 54 M increments/s
```
很明显的可以看出，threads的增加，反而导致系统的吞吐量下降。

[approximate_counter.c](approximate_counter.c)

根据书中的版本实现的approximate counter。需要先根据自己的cpu核心数修改源码。

编译可以用下面的命令关闭编译器优化：
```shell
gcc -O0 -pthread approximate_counter.c -o approximate_counter
```

测试方法，两种，先固定threshold，比如1024，然后分别增加线程数，观测数字：
```shell
./approximate_counter 1 1024
./approximate_counter 2 1024
./approximate_counter 4 1024
./approximate_counter 8 1024
./approximate_counter 16 1024
./approximate_counter 32 1024
```
然后，固定线程数，比如8，测量threshold变化导致的数据变化：
```shell
./approximate_counter 8 1
./approximate_counter 8 10
./approximate_counter 8 100
./approximate_counter 8 1000
./approximate_counter 8 10000
```