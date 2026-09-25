# `std::shared_mutex` vs `std::mutex`

I wanted to valuate whether `std::shared_mutex` provides better scalability than `std::mutex` for read-heavy workloads.

## Benchmark Setup

Two stack implementations were compared:

* `std::mutex` + `std::lock_guard`
* `std::shared_mutex` + `std::shared_lock` / `std::unique_lock`

The benchmark focused on the `top()` operation, which only reads data. Multiple reader threads repeatedly called `top()` while the stack contents remained unchanged.

## Initial Observation

A benchmark performing only `top()` showed that `std::shared_mutex` was slower than `std::mutex`. This is expected. The protected work is extremely small, so the benchmark primarily measures lock acquisition and release overhead. Since `std::shared_mutex` is more complex than `std::mutex`, its uncontended cost is typically higher.:

```cpp
benchmark::DoNotOptimize(s.top());
```

## Read-Heavy Scenario

After adding some work while holding the lock (simulating a realistic read-side critical section), the results changed significantly.

```cpp
auto value = s.top();

for (int i = 0; i < 100; ++i)
{
    benchmark::DoNotOptimize(i);
}
```

With multiple reader threads, `std::shared_mutex` began to outperform `std::mutex`.

### `std::mutex`

Readers serialize:

```text
T1 -> read
T2 -> wait
T3 -> wait
T4 -> wait
```

Only one thread can access the protected region at a time.

### `std::shared_mutex`

Readers execute concurrently:

```text
T1 -> read
T2 -> read
T3 -> read
T4 -> read
```

Multiple threads can hold a shared lock simultaneously.

## Key Takeaway

`std::shared_mutex` is **not automatically faster** than `std::mutex`.

For very small critical sections:

* `std::mutex` is often faster
* `std::shared_mutex` adds extra overhead

For read-heavy workloads with:

* many concurrent readers
* infrequent writers
* non-trivial work inside the critical section [!!]

`std::shared_mutex` can provide significantly better scalability.

## Practical Rule

Use `std::shared_mutex` when:

* reads greatly outnumber writes
* readers spend meaningful time inside the critical section

Use `std::mutex` when:

* critical sections are extremely short
* contention is low
* simplicity and minimal overhead are preferred

## Benchmark Results

```bash
Running ./mutex_stack
Run on (12 X 4104.01 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x6)
  L1 Instruction 32 KiB (x6)
  L2 Unified 256 KiB (x6)
  L3 Unified 12288 KiB (x1)
Load Average: 0.06, 0.03, 0.00
------------------------------------------------------------------------
Benchmark                              Time             CPU   Iterations
------------------------------------------------------------------------
BM_mutex_top/64/threads:1            235 ns          235 ns      2925996
BM_mutex_top/64/threads:2            294 ns          293 ns      2286262
BM_mutex_top/64/threads:4            502 ns          461 ns      1473964
BM_mutex_top/64/threads:8           1321 ns         1097 ns       694600
------------------------------------------------------------------------
BM_mutex_stack/1024/threads:1      32811 ns        32811 ns        21343 items_per_second=31.2094M/s
BM_mutex_stack/1024/threads:2     316519 ns       309578 ns         2188 items_per_second=6.61545M/s
BM_mutex_stack/1024/threads:4     448037 ns       336186 ns         2056 items_per_second=12.1837M/s
BM_mutex_stack/1024/threads:8    1317090 ns       937966 ns          800 items_per_second=8.73379M/s
```

```bash
Running ./readwrite_stack
Run on (12 X 4104.01 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x6)
  L1 Instruction 32 KiB (x6)
  L2 Unified 256 KiB (x6)
  L3 Unified 12288 KiB (x1)
Load Average: 0.25, 0.08, 0.02
-----------------------------------------------------------------------------
Benchmark                                   Time             CPU   Iterations
-----------------------------------------------------------------------------
BM_rw_top/64/threads:1                    237 ns          237 ns      2874433
BM_rw_top/64/threads:2                    323 ns          323 ns      2141068
BM_rw_top/64/threads:4                    346 ns          346 ns      1686480
BM_rw_top/64/threads:8                    711 ns          711 ns       968696
-----------------------------------------------------------------------------
BM_readwrite_stackk/1024/threads:1      44832 ns        44832 ns        15483 items_per_second=22.8406M/s
BM_readwrite_stackk/1024/threads:2     464654 ns       458591 ns         1480 items_per_second=4.46586M/s
BM_readwrite_stackk/1024/threads:4    1122054 ns      1012518 ns          700 items_per_second=4.04536M/s
BM_readwrite_stackk/1024/threads:8    2659988 ns      2209021 ns          424 items_per_second=3.70843M/s
```

## Build and Run
```bash
$ g++ -std=c++20 -g -O3 -mavx2 -Wall -pedantic -I$GBENCH_DIR/include mutex_stack.cpp $GBENCH_DIR/build/src/libbenchmark.a -pthread -lrt -lm -o mutex_stack
$ ./mutex_stack

$ g++ -std=c++20 -g -O3 -mavx2 -Wall -pedantic -I$GBENCH_DIR/include readwrite_stack.cpp $GBENCH_DIR/build/src/libbenchmark.a -pthread -lrt -lm -o readwrite_stack
$ ./readwrite_stack
```
