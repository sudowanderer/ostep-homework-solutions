## syscall_cost.c

主要测量的是
```shell
用户态
  ↓
系统调用入口
  ↓
内核参数检查
  ↓
系统调用返回
  ↓
用户态
```


## [timer_precision.c](timer_precision.c)

这里主要测量gettimeofday函数的误差和调用耗时。可以明显观测到，调用耗时误差是微妙us级别的。而read systemcall 是ns级别的。1us=1000ns。
这里差了1000倍。


## [02context_switch.c](02context_switch.c)

这里测量一次完整的线程切换，所需要的耗时，注意是，一个来回。**代码只能在Linux上编译并运行**，macOS会报错。因为macOS没有实现sched_setaffinity，macos的Scheduler会自己决定每个process跑在哪个cpu上。

测量结果：

> Warm-up iterations: 10000
Measured iterations: 100000
Total measured time: 841438.000 us
Average round-trip time: 8.414 us
Approximate time per switch: 4.207 us

Note: the per-switch value also contains pipe read/write,
system-call and scheduler overhead. It is not a pure
context-switch measurement.


可以看到，一次上下文切换是4.2us微秒，和systemcall纳秒不是一个数量级。耗时会更多。大概差一个数量级。也就是10倍。

这里列举一个经典的延迟金字塔:

```shell
普通函数调用
    ↓
几 ns

System Call
    ↓
几十~几百 ns

Context Switch
    ↓
几 us

磁盘 I/O
    ↓
几 ms

网络
    ↓
几十~几百 ms
```
另外，这里有个细节，context switch中，耗时占比最高的是**_保存上下文和恢复上下文_**。而不是Scheduler等。
