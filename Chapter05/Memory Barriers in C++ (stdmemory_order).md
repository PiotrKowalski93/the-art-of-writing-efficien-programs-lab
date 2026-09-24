## Memory Barriers in C++ (std::memory_order)

Modern CPUs and compilers aggressively reorder memory operations for performance.
Without proper synchronization, different threads may observe memory in different orders.
```std::atomic``` provides several memory ordering models to control this behavior.

1. ```memory_order_relaxed```
- Atomic read/write
- No data race
- No synchronization
- No ordering guarantees

Threads may observe operations in different order.

```cpp
#include <atomic>
#include <thread>
#include <iostream>

std::atomic<int> counter = 0;

void worker()
{
    for(int i = 0; i < 100000; ++i)
    {
        counter.fetch_add(1, std::memory_order_relaxed);
    }
}

int main()
{
    std::thread t1(worker);
    std::thread t2(worker);

    t1.join();
    t2.join();

    std::cout << counter << "\n";
}
```

2. ```memory_order_release```

A release operation guarantees that all previous writes become visible before the atomic store.
It is used to publish data to other threads.

```cpp
#include <atomic>

std::atomic<bool> ready(false);
int data = 0;

void producer()
{
    data = 42;

    // publish data
    ready.store(true, std::memory_order_release);
}
```

3. ```memory_order_acquire```

An acquire operation guarantees that after observing a released value, all previous writes from the producer become visible.
Acquire pairs with release.

```cpp
#include <atomic>
#include <iostream>

std::atomic<bool> ready(false);
int data = 0;

void consumer()
{
    if(ready.load(std::memory_order_acquire))
    {
        // guaranteed to see data == 42
        std::cout << data << "\n";
    }
}
```

4. ```memory_order_seq_cst```

Sequential consistency (seq_cst) provides acquire/release semantics plus a global ordering of atomic operations.
All threads observe atomic operations in the same order.

```cpp
#include <atomic>

std::atomic<int> x(0);

void writer()
{
    x.store(1, std::memory_order_seq_cst);
}

void reader()
{
    int value = x.load(std::memory_order_seq_cst);
}
```

## Final Takeaway

Modern CPUs reorder memory operations aggressively.
Memory barriers exist to:
- control visibility between threads
- prevent dangerous reordering
- safely synchronize shared state without locks