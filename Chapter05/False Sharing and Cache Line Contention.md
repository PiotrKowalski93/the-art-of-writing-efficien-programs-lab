## False Sharing and Cache Line Contention

```bash
$ clang++-17 -g -O3 -mavx2 -Wall -pedantic -I$GBENCH_DIR/include memory_sync_cost.cpp  $GBENCH_DIR/build/src/libbenchmark.a -pthread -lrt -lm -o memory_sync_cost
$ ./memory_sync_cost 
2026-05-01T09:43:55+02:00
Running ./memory_sync_cost
Run on (12 X 4800 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x6)
  L1 Instruction 32 KiB (x6)
  L2 Unified 256 KiB (x6)
  L3 Unified 12288 KiB (x1)
Load Average: 0.68, 0.70, 0.60
***WARNING*** CPU scaling is enabled, the benchmark real time measurements may be noisy and will incur extra overhead.
------------------------------------------------------------------------------------------
Benchmark                                Time             CPU   Iterations UserCounters...
------------------------------------------------------------------------------------------
BM_shared/1024/threads:1              1200 ns         1200 ns       540921 items_per_second=853.448M/s
BM_shared/1024/threads:2              1244 ns         1243 ns       563436 items_per_second=1.64717G/s
BM_shared/1024/threads:4              1333 ns         1333 ns       544920 items_per_second=3.07325G/s
BM_shared/1024/threads:8              1478 ns         1336 ns       532888 items_per_second=6.13305G/s
BM_shared/1024/threads:12             1502 ns         1340 ns       518388 items_per_second=9.16971G/s
------------------------------------------------------------------------------------------
BM_false_shared/1024/threads:1        1194 ns         1194 ns       575476 items_per_second=857.78M/s
BM_false_shared/1024/threads:2        1730 ns         1730 ns       343274 items_per_second=1.18369G/s
BM_false_shared/1024/threads:4        2026 ns         2024 ns       355120 items_per_second=2.02344G/s
BM_false_shared/1024/threads:8        3424 ns         3424 ns       196296 items_per_second=2.39246G/s
BM_false_shared/1024/threads:12       4880 ns         4222 ns       121320 items_per_second=2.91053G/s
------------------------------------------------------------------------------------------
BM_local/1024/threads:1               1193 ns         1193 ns       557698 items_per_second=858.514M/s
BM_local/1024/threads:2               1209 ns         1209 ns       573382 items_per_second=1.69335G/s
BM_local/1024/threads:4               1298 ns         1298 ns       400000 items_per_second=3.15508G/s
BM_local/1024/threads:8               1583 ns         1456 ns       533224 items_per_second=5.62569G/s
BM_local/1024/threads:12              1630 ns         1484 ns       464928 items_per_second=8.2826G/s
```

This benchmark demonstrates how dramatically performance changes depending on whether multiple threads modify data located in the same cache line.

The goal was to compare three scenarios:

- Local variable (BM_local)
    - Each thread increments its own stack variable.
    - No cache contention.
    - Best possible case.
- False sharing (BM_false_shared)
    - Each thread updates a different array element, but those elements are adjacent in memory and end up in the same cache line.
    - Threads invalidate each other’s cache lines constantly.
- Separated cache lines (BM_shared)
    - Each thread writes to a different memory location spaced far enough apart.
    - Prevents cache line sharing between cores.

Modern CPUs do not synchronize individual variables between cores. They synchronize entire **cache lines**.
Typical cache line size: 64B. Even though threads modify different variables, the CPU cache coherence protocol treats the entire line as shared.

## What Is False Sharing?

False sharing occurs when multiple threads modify different variables, but those variables reside in the same cache line.

**Example:**

```bash
unsigned long arr[12];

Thread 0 -> arr[0]
Thread 1 -> arr[1]
Thread 2 -> arr[2]
```

Even though variables are independent:

- Thread 0 invalidates cache line
- Thread 1 invalidates it back
- Thread 2 invalidates again

This creates heavy cache coherency traffic.
The result is:
- pipeline stalls
- cache invalidations
- memory ordering overhead
- massive scalability loss