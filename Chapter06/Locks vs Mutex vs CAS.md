## Lock Contention Benchmark — mutex vs atomic vs CAS

This benchmark compares synchronization primitives under increasing thread contention:
- std::mutex
- std::atomic with:
- memory_order_seq_cst
- memory_order_relaxed
- compare_exchange_strong
- compare_exchange_weak

```cpp
$ ./locks 
2026-05-05T20:53:20+02:00
Running ./locks
Run on (12 X 4800 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x6)
  L1 Instruction 32 KiB (x6)
  L2 Unified 256 KiB (x6)
  L3 Unified 12288 KiB (x1)
Load Average: 1.48, 0.53, 0.19
***WARNING*** CPU scaling is enabled, the benchmark real time measurements may be noisy and will incur extra overhead.
----------------------------------------------------------------------------------
Benchmark                                        Time             CPU   Iterations
----------------------------------------------------------------------------------
BM_mutex/real_time/threads:1                  4.69 ns         4.69 ns    154535826
BM_mutex/real_time/threads:2                   120 ns          120 ns      5604924
BM_mutex/real_time/threads:4                   188 ns          166 ns      3236332
BM_mutex/real_time/threads:8                   501 ns          437 ns      1450744
BM_mutex/real_time/threads:16                  960 ns          532 ns       805424
----------------
BM_atomic_seq_cast/real_time/threads:1        4.02 ns         4.02 ns    175340369
BM_atomic_seq_cast/real_time/threads:2        20.2 ns         20.2 ns     38608904
BM_atomic_seq_cast/real_time/threads:4        57.1 ns         57.1 ns     12106724
BM_atomic_seq_cast/real_time/threads:8         101 ns         99.9 ns      7499776
BM_atomic_seq_cast/real_time/threads:16        179 ns         93.2 ns      4293616
----------------
BM_atomic_relaxed/real_time/threads:1         3.94 ns         3.93 ns    171821430
BM_atomic_relaxed/real_time/threads:2         18.6 ns         18.6 ns     39237682
BM_atomic_relaxed/real_time/threads:4         53.1 ns         53.1 ns     14944232
BM_atomic_relaxed/real_time/threads:8         69.5 ns         56.6 ns      7784368
BM_atomic_relaxed/real_time/threads:16         197 ns          145 ns      4988080
----------------
BM_CAS_strong/real_time/threads:1             6.65 ns         6.65 ns    101420772
BM_CAS_strong/real_time/threads:2             77.3 ns         77.3 ns     18798874
BM_CAS_strong/real_time/threads:4              133 ns          129 ns      5547604
BM_CAS_strong/real_time/threads:8              340 ns          299 ns      1826840
BM_CAS_strong/real_time/threads:16             962 ns          733 ns       795936
----------------
BM_CAS_weak/real_time/threads:1               6.49 ns         6.49 ns     97209354
BM_CAS_weak/real_time/threads:2               67.4 ns         67.4 ns     15555738
BM_CAS_weak/real_time/threads:4                161 ns          161 ns      6700024
BM_CAS_weak/real_time/threads:8                312 ns          259 ns      2133128
BM_CAS_weak/real_time/threads:16               847 ns          621 ns       831248
```

### Results

| Threads |   Mutex | Atomic SeqCst | Atomic Relaxed | CAS Strong | CAS Weak |
| ------- | ------: | ------------: | -------------: | ---------: | -------: |
| 1       | 4.69 ns |       4.02 ns |        3.94 ns |    6.65 ns |  6.49 ns |
| 2       |  120 ns |       20.2 ns |        18.6 ns |    77.3 ns |  67.4 ns |
| 4       |  188 ns |       57.1 ns |        53.1 ns |     133 ns |   161 ns |
| 8       |  501 ns |        101 ns |        69.5 ns |     340 ns |   312 ns |
| 16      |  960 ns |        179 ns |         197 ns |     962 ns |   847 ns |

## The Real Clue

The benchmark is not really measuring:
```"How fast is mutex vs atomic?"```

It is measuring cache contention.

All threads modify the SAME shared variable.

This means:
- only one core can own the cache line in Modified state
- ownership constantly migrates between cores
- MESI protocol traffic dominates cost

As thread count rises:
- synchronization cost rises
- cache invalidation rises
- retries rise
- scalability collapses

**This is the true bottleneck.**

## Takeaway

The biggest lesson is:
- Modern multicore performance is often limited more by cache coherency than by raw computation.

Synchronization primitives ultimately fight over:
- cache line ownership
- memory ordering
- coherency traffic
