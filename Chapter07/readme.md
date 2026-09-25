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
* non-trivial work inside the critical section

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
Load Average: 0.18, 0.17, 0.10
------------------------------------------------------------------------
Benchmark                              Time             CPU   Iterations
------------------------------------------------------------------------
BM_mutex_top/64/threads:1            236 ns          236 ns      2953072
BM_mutex_top/64/threads:2            329 ns          328 ns      2192556
BM_mutex_top/64/threads:4            402 ns          386 ns      1698252
BM_mutex_top/64/threads:8           1210 ns         1014 ns       759224
------------------------------------------------------------------------
BM_mutex_stack/1024/threads:1      32407 ns        32407 ns        21510 items_per_second=31.5983M/s
BM_mutex_stack/1024/threads:2     273163 ns       266341 ns         2674 items_per_second=7.68939M/s
BM_mutex_stack/1024/threads:4     410795 ns       314698 ns         2112 items_per_second=13.0157M/s
BM_mutex_stack/1024/threads:8    1379891 ns      1021122 ns          968 items_per_second=8.02255M/s
```

```bash
Running ./readwrite_stack
Run on (12 X 4104.01 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x6)
  L1 Instruction 32 KiB (x6)
  L2 Unified 256 KiB (x6)
  L3 Unified 12288 KiB (x1)
Load Average: 0.22, 0.18, 0.10
-----------------------------------------------------------------------------
Benchmark                                   Time             CPU   Iterations
-----------------------------------------------------------------------------
BM_rw_top/64/threads:1                    369 ns          369 ns      1875223
BM_rw_top/64/threads:2                    421 ns          421 ns      1648220
BM_rw_top/64/threads:4                    484 ns          484 ns      1459436
BM_rw_top/64/threads:8                    632 ns          632 ns      1042632
-----------------------------------------------------------------------------
BM_readwrite_stackk/1024/threads:1      45458 ns        45457 ns        15224 items_per_second=22.5266M/s
BM_readwrite_stackk/1024/threads:2     435153 ns       429612 ns         1480 items_per_second=4.76709M/s
BM_readwrite_stackk/1024/threads:4    1092798 ns       986059 ns          900 items_per_second=4.15391M/s
BM_readwrite_stackk/1024/threads:8    2552594 ns      2099475 ns          464 items_per_second=3.90193M/s
```

## Build and Run
```bash
$ g++ -std=c++20 -g -O3 -mavx2 -Wall -pedantic -I$GBENCH_DIR/include mutex_stack.cpp $GBENCH_DIR/build/src/libbenchmark.a -pthread -lrt -lm -o mutex_stack
$ ./mutex_stack

$ g++ -std=c++20 -g -O3 -mavx2 -Wall -pedantic -I$GBENCH_DIR/include readwrite_stack.cpp $GBENCH_DIR/build/src/libbenchmark.a -pthread -lrt -lm -o readwrite_stack
$ ./readwrite_stack
```
