## Memory Synchronization Cost Benchmark

This benchmark compares the cost of incrementing a shared counter using three approaches:

- no synchronization (++x)
- std::atomic
- std::mutex

The goal was to observe how synchronization overhead scales with thread contention.

```bash
$ clang++-17 -g -O3 -mavx2 -Wall -pedantic -I$GBENCH_DIR/include memory_sync_cost.cpp  $GBENCH_DIR/build/src/libbenchmark.a -pthread -lrt -lm -o memory_sync_cost
$ ./memory_sync_cost 
2026-04-30T14:27:37+02:00
Running ./memory_sync_cost
Run on (12 X 4800 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x6)
  L1 Instruction 32 KiB (x6)
  L2 Unified 256 KiB (x6)
  L3 Unified 12288 KiB (x1)
Load Average: 0.61, 0.52, 0.37
***WARNING*** CPU scaling is enabled, the benchmark real time measurements may be noisy and will incur extra overhead.
------------------------------------------------------------------------------
Benchmark                                    Time             CPU   Iterations
------------------------------------------------------------------------------
BM_simple_increment/1024/threads:1        37.4 ns         37.4 ns     18464056
BM_simple_increment/1024/threads:2        53.5 ns         53.4 ns     12936502
BM_simple_increment/1024/threads:4        66.1 ns         66.1 ns     10456020
BM_simple_increment/1024/threads:8         178 ns          163 ns      4287472
BM_simple_increment/1024/threads:12        231 ns          194 ns      3199536
BM_atomic_increment/1024/threads:1         123 ns          123 ns      5420670
BM_atomic_increment/1024/threads:2         865 ns          865 ns       736016
BM_atomic_increment/1024/threads:4        2111 ns         1978 ns       364368
BM_atomic_increment/1024/threads:8        3251 ns         2892 ns       222472
BM_atomic_increment/1024/threads:12       5190 ns         4058 ns       125340
BM_mutex_increment/1024/threads:1          396 ns          396 ns      1765623
BM_mutex_increment/1024/threads:2         1889 ns         1889 ns       341250
BM_mutex_increment/1024/threads:4         3362 ns         3361 ns       201924
BM_mutex_increment/1024/threads:8         5209 ns         5208 ns       134000
BM_mutex_increment/1024/threads:12        5826 ns         5365 ns       124824
```

### Key observations

- No synchronization is by far the fastest approach because there is no cache coherency enforcement or locking involved.
  Performance scales reasonably well until higher thread counts. However, this version is incorrect in multithedreaded code because it introduces a data race and results in undefined behavior.
- **std::atomic** introduces a significant overhead compared to unsynchronized access. Even with only 2 threads, latency jumps from ~53 ns to ~865 ns.
  The cost comes from maintaining cache coherency between CPU cores. Multiple threads continuously invalidate and transfer ownership of the same cache line ("cache line ping-pong").
- **std::mutex** is the most expensive synchronization mechanism. Besides cache synchronization, mutexes also require lock management and thread coordination.
  Under contention, scalability degrades quickly.

### Main takeaway

Synchronization is not free.

For shared writable data:
- unsynchronized access is fastest but incorrect
- atomics are cheaper than mutexes for simple operations
- mutexes provide stronger guarantees but with the highest contention cost.

This benchmark also demonstrates how modern CPUs spend significant time coordinating cache ownership between cores when multiple threads modify the same memory location.