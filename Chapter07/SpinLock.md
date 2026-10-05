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