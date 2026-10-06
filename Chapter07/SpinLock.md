# Spinlock — low-level implementation notes

## Overview

A spinlock is a synchronization primitive where a thread repeatedly tries to acquire a lock instead of immediately blocking/sleeping.

This can be very effective when the critical section is extremely short, because the cost of context switching or putting a thread to sleep may be larger than simply
waiting for a few CPU cycles.

A simple implementation might look like:

```cpp
class SpinLock {
public:
    void lock() {
        while (lock_.exchange(1, std::memory_order_acquire)) {
        }
    }

    void unlock() {
        lock_.store(0, std::memory_order_release);
    }

private:
    std::atomic<int> lock_{0};
};
````

However, this implementation can generate a lot of expensive atomic RMW operations under contention.

## Test-Test-And-Set

A more efficient approach is to first perform a cheap `load()` and only perform the expensive `exchange()` when the lock appears to be free.

```cpp
lock_.load(std::memory_order_relaxed) || lock_.exchange(1, std::memory_order_acquire)
```

Why not just `exchange()`? `exchange()` is a read-modify-write operation.
Under contention, many CPUs repeatedly executing:

```cpp
lock_.exchange(1, std::memory_order_acquire);
```
can cause significant cache-coherence traffic because the cache line containing the lock has to be acquired for modification.
Instead, while the lock is held, all contenders can repeatedly perform:

```cpp
lock_.load(std::memory_order_relaxed);
```

Only when a thread observes `0` does it attempt the RMW operation. This reduces unnecessary cache-line contention.

## Manual loop unrolling

The interesting part of this implementation is that the spin loop is manually unrolled.

Instead of:

```cpp
for (;;) {
    if (!try_lock())
        return;

    spin_wait_short_sleep();
}
```
we perform several lock attempts explicitly:

```cpp
lock_.load(std::memory_order_relaxed) || lock_.exchange(1, std::memory_order_acquire)
```

The purpose is not simply to reduce the number of branches. There is a more subtle microarchitectural reason.

### Reorder Buffer (ROB) exhaustion

Modern out-of-order CPUs contain a **Reorder Buffer (ROB)**. The CPU can execute instructions speculatively and out of order, but instructions
must eventually retire in program order.

Consider a spinlock where an atomic operation has a relatively long latency because it is waiting for cache-coherence traffic:
The CPU does not necessarily stop immediately when the atomic operation stalls. It can speculatively decode and execute subsequent iterations of the loop.
Without unrolling, every iteration introduces additional loop-control instructions:

```cpp
++spin_count;
conditional branch;
```

Those instructions consume ROB entries even though they do not contribute directly to acquiring the lock. Eventually the ROB can become full
At that point the frontend cannot continue making useful speculative progress.

With an unrolled loop we remove much of the repeated loop-control overhead.

Instead of:

```text
try
increment
branch
try
increment
branch
try
increment
branch
...
```

the CPU sees more like:

```text
try
try
try
try
try
try
try
try
increment
branch
```

This means that during speculative execution, more ROB capacity can be used for actual lock attempts rather than predictable loop-control instructions.
The goal is to allow the CPU to track more speculative lock attempts before the pipeline runs out of ROB capacity.
This is a very low-level optimization and its usefulness depends heavily on the CPU microarchitecture and the workload.

## Backoff

Spinning forever is also undesirable. If the lock is held for longer, continuously hammering the atomic operation wastes
CPU resources and increases contention.

Therefore the implementation uses:

```cpp
if (spin_count < 8) {
    spin_wait_short_sleep();
} else {
    spin_count = 0;
    spin_wait_long_sleep();
}
```

The exact numbers are tuning parameters. mThere is no universal rule that says `8 attempts = optimal`

## Why this can be faster than a mutex

For a very short critical section:

```text
lock()
   │
   ├── few dozen CPU cycles
   │
unlock()
```

a spinlock may be faster because the waiting thread remains on the CPU.

A blocking mutex may involve:

```text
lock()
   ↓
kernel / scheduler interaction
   ↓
thread blocked
   ↓
context switch
   ↓
wake-up
   ↓
scheduler
   ↓
thread runs again
```

That overhead can be much larger than simply waiting for a short critical section to finish.
However, spinlocks are dangerous when the critical section is long or contention is high because they can waste entire CPU cores.


```bash
Kowal@DESKTOP-9LDCA2E:~/source/the-art-of-writing-efficien-programs-lab/Chapter07$ ./spinlock
2026-10-06T22:24:50+02:00
Running ./spinlock
Run on (12 X 4104.01 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x6)
  L1 Instruction 32 KiB (x6)
  L2 Unified 256 KiB (x6)
  L3 Unified 12288 KiB (x1)
Load Average: 0.03, 0.03, 0.04
-------------------------------------------------------------------------------------------------------------------------------------------
Benchmark                                                                   Time             CPU   Iterations UserCounters...
-------------------------------------------------------------------------------------------------------------------------------------------
BM_SpinLock_Unrolled_Contention/threads:2                                12.1 ns         6.38 ns    109392690 items_per_second=313.47M/s
BM_SpinLock_Unrolled_Contention/threads:4                                23.1 ns         6.47 ns    106857220 items_per_second=618.275M/s
BM_SpinLock_Unrolled_Contention/threads:8                                42.7 ns         6.62 ns    101997512 items_per_second=1.20882G/s
BM_SpinLock_Unrolled_Contention/threads:12                               65.6 ns         6.89 ns    100749384 items_per_second=1.74111G/s
BM_SpinLock_Unrolled_Contention/threads:16                               89.1 ns         7.27 ns     96566960 items_per_second=2.19975G/s
BM_SpinLock_Unrolled_Contention/threads:20                                115 ns         7.35 ns     91523420 items_per_second=2.72219G/s
-------------------------------------------------------------------------------------------------------------------------------------------
BM_SpinLock_Loop_Contention/threads:2                                    12.0 ns         6.76 ns    102710682 items_per_second=295.702M/s
BM_SpinLock_Loop_Contention/threads:4                                    23.3 ns         7.78 ns     89660368 items_per_second=514.144M/s
BM_SpinLock_Loop_Contention/threads:8                                    49.3 ns         10.7 ns     64939360 items_per_second=744.982M/s
BM_SpinLock_Loop_Contention/threads:12                                   82.5 ns         15.1 ns     47224896 items_per_second=794.098M/s
BM_SpinLock_Loop_Contention/threads:16                                    121 ns         28.6 ns     26081504 items_per_second=558.915M/s
BM_SpinLock_Loop_Contention/threads:20                                    162 ns         35.1 ns     24657180 items_per_second=569.924M/s
-------------------------------------------------------------------------------------------------------------------------------------------
BM_SpinLock_Unrolled_Contention_longer_critical_section/threads:2         185 ns          124 ns      5656964 items_per_second=16.1716M/s
BM_SpinLock_Unrolled_Contention_longer_critical_section/threads:4         335 ns          125 ns      5590988 items_per_second=31.9314M/s
BM_SpinLock_Unrolled_Contention_longer_critical_section/threads:8         724 ns          128 ns      5334656 items_per_second=62.3351M/s
BM_SpinLock_Unrolled_Contention_longer_critical_section/threads:12        982 ns          130 ns      5283996 items_per_second=92.0798M/s
BM_SpinLock_Unrolled_Contention_longer_critical_section/threads:16       1797 ns          140 ns      1600000 items_per_second=114.031M/s
BM_SpinLock_Unrolled_Contention_longer_critical_section/threads:20       2328 ns          148 ns      2000000 items_per_second=135.222M/s
-------------------------------------------------------------------------------------------------------------------------------------------
BM_SpinLock_Loop_Contention_longer_critical_section/threads:2             188 ns          126 ns      5454812 items_per_second=15.8215M/s
BM_SpinLock_Loop_Contention_longer_critical_section/threads:4             342 ns          131 ns      5343816 items_per_second=30.4584M/s
BM_SpinLock_Loop_Contention_longer_critical_section/threads:8             631 ns          140 ns      4928184 items_per_second=57.0546M/s
BM_SpinLock_Loop_Contention_longer_critical_section/threads:12            988 ns          152 ns      4618728 items_per_second=78.727M/s
BM_SpinLock_Loop_Contention_longer_critical_section/threads:16           1157 ns          172 ns      3851040 items_per_second=92.816M/s
BM_SpinLock_Loop_Contention_longer_critical_section/threads:20           1426 ns          194 ns      2446260 items_per_second=103.254M/s
```