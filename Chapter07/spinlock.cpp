#include <atomic>
#include <time.h>
#include <immintrin.h>
#include <benchmark/benchmark.h>

static const struct timespec spin_wait_short = { 0, 1 };
static const struct timespec spin_wait_long  = { 0, 10000001 };
static inline void spin_wait_short_sleep() { nanosleep(&spin_wait_short, nullptr); }
static inline void spin_wait_long_sleep()  { nanosleep(&spin_wait_long,  nullptr); }

// Designed for high contention environment
// TEST-TEST-and-SET
class SpinLock {
public:
    // Iterations:        100
    // Instructions:      8700
    // Total Cycles:      6292
    // Total uOps:        11200
    // Dispatch Width:    4
    // uOps Per Cycle:    1.78
    // IPC:               1.38
    // Block RThroughput: 28.0
    void lock() 
    {
        // Why we manually urnoll loop??
        // When it stalls at the head of the ROB, the CPU will speculatively decode and execute subsequent iterations of the loop. 
        // If the loop is not unrolled, the ROB rapidly fills up with loop counter increments (++spin_count) and conditional branch instructions...
        for(int spin_count = 0; lock_.load(std::memory_order_relaxed) || lock_.exchange(1, std::memory_order_acquire); ++spin_count)
        {
            if(!(lock_.load(std::memory_order_relaxed) || lock_.exchange(1, std::memory_order_acquire))) return;
            if(!(lock_.load(std::memory_order_relaxed) || lock_.exchange(1, std::memory_order_acquire))) return;
            if(!(lock_.load(std::memory_order_relaxed) || lock_.exchange(1, std::memory_order_acquire))) return;
            if(!(lock_.load(std::memory_order_relaxed) || lock_.exchange(1, std::memory_order_acquire))) return;
            if(!(lock_.load(std::memory_order_relaxed) || lock_.exchange(1, std::memory_order_acquire))) return;
            if(!(lock_.load(std::memory_order_relaxed) || lock_.exchange(1, std::memory_order_acquire))) return;
            if(!(lock_.load(std::memory_order_relaxed) || lock_.exchange(1, std::memory_order_acquire))) return;
            if(!(lock_.load(std::memory_order_relaxed) || lock_.exchange(1, std::memory_order_acquire))) return;

            if(spin_count < 8) {
                spin_wait_short_sleep();
            } else {
                spin_count = 0;
                spin_wait_long_sleep();
            }
        }
    };

    void lock_mm_pause()
    {
        // Why we manually urnoll loop??
        // When it stalls at the head of the ROB, the CPU will speculatively decode and execute subsequent iterations of the loop. 
        // If the loop is not unrolled, the ROB rapidly fills up with loop counter increments (++spin_count) and conditional branch instructions...
        for(int spin_count = 0; lock_.load(std::memory_order_relaxed) || lock_.exchange(1, std::memory_order_acquire); ++spin_count)
        {
            if(!(lock_.load(std::memory_order_relaxed) || lock_.exchange(1, std::memory_order_acquire))) return;
            if(!(lock_.load(std::memory_order_relaxed) || lock_.exchange(1, std::memory_order_acquire))) return;
            if(!(lock_.load(std::memory_order_relaxed) || lock_.exchange(1, std::memory_order_acquire))) return;
            if(!(lock_.load(std::memory_order_relaxed) || lock_.exchange(1, std::memory_order_acquire))) return;
            if(!(lock_.load(std::memory_order_relaxed) || lock_.exchange(1, std::memory_order_acquire))) return;
            if(!(lock_.load(std::memory_order_relaxed) || lock_.exchange(1, std::memory_order_acquire))) return;
            if(!(lock_.load(std::memory_order_relaxed) || lock_.exchange(1, std::memory_order_acquire))) return;
            if(!(lock_.load(std::memory_order_relaxed) || lock_.exchange(1, std::memory_order_acquire))) return;

            if(spin_count < 8) {
                _mm_pause();
            } else {
                spin_count = 0;
                _mm_pause();
                _mm_pause();
                _mm_pause();
            }
        }
    };

    // Iterations:        100
    // Instructions:      3400
    // Total Cycles:      2607
    // Total uOps:        4200
    // Dispatch Width:    4
    // uOps Per Cycle:    1.61
    // IPC:               1.30
    // Block RThroughput: 10.5
    void lock_not_unrolled() 
    {
        for(int spin_count = 0; lock_.load(std::memory_order_relaxed) || lock_.exchange(1, std::memory_order_acquire); ++spin_count)
        {
            if(spin_count < 64) {
                spin_wait_short_sleep();
            } else {
                spin_count = 0;
                spin_wait_long_sleep();
            }
        }
    };

    bool try_lock()
    {
        for (int spin_count = 0; lock_.load(std::memory_order_relaxed) || lock_.exchange(1, std::memory_order_acquire); ++spin_count)
        {
            if (!(lock_.load(std::memory_order_relaxed) || lock_.exchange(1, std::memory_order_acquire))) return true;
            if (!(lock_.load(std::memory_order_relaxed) || lock_.exchange(1, std::memory_order_acquire))) return true;
            if (!(lock_.load(std::memory_order_relaxed) || lock_.exchange(1, std::memory_order_acquire))) return true;
            if (!(lock_.load(std::memory_order_relaxed) || lock_.exchange(1, std::memory_order_acquire))) return true;
            if (!(lock_.load(std::memory_order_relaxed) || lock_.exchange(1, std::memory_order_acquire))) return true;
            if (!(lock_.load(std::memory_order_relaxed) || lock_.exchange(1, std::memory_order_acquire))) return true;
            if (!(lock_.load(std::memory_order_relaxed) || lock_.exchange(1, std::memory_order_acquire))) return true;
            
            if (spin_count < 8) {
                spin_wait_short_sleep();
            } else {
                return false;
            }
        }

        return true;
    }

    void unlock()
    {
        lock_.store(0, std::memory_order_release); 
    }

    // We can use relaxed because we just check if lock is locked.
    bool locked() const
    {
        return lock_.load(std::memory_order_relaxed) == 1;
    }

private:
    std::atomic<int> lock_{0};
};

static SpinLock shared_unrolled_lock;
static std::uint64_t shared_unrolled_counter = 0;

static void BM_SpinLock_Unrolled_Contention(benchmark::State& state)
{
    benchmark::DoNotOptimize(&shared_unrolled_counter);

    for (auto _ : state)
    {
        shared_unrolled_lock.lock();
        ++shared_unrolled_counter;
        benchmark::ClobberMemory();
        shared_unrolled_lock.unlock();
    }

    state.SetItemsProcessed(state.iterations());
}

static SpinLock shared_lock;
static std::uint64_t shared_loop_counter = 0;

static void BM_SpinLock_Loop_Contention(benchmark::State& state)
{
    benchmark::DoNotOptimize(&shared_loop_counter);

    for (auto _ : state)
    {
        shared_lock.lock_not_unrolled();
        ++shared_loop_counter;
        benchmark::ClobberMemory();
        shared_lock.unlock();
    }

    state.SetItemsProcessed(state.iterations());
}

static SpinLock shared_unrolled_lock_longer_critical_section;
static std::uint64_t shared_unrolled_counter_longer_critical_section = 0;

static void BM_SpinLock_Unrolled_Contention_longer_critical_section(benchmark::State& state)
{
    benchmark::DoNotOptimize(&shared_unrolled_counter_longer_critical_section);

    for (auto _ : state)
    {
        shared_unrolled_lock_longer_critical_section.lock();

        for (int i = 0; i < 30; ++i)
        {
            _mm_pause();
            ++shared_unrolled_counter_longer_critical_section;
        }

        benchmark::ClobberMemory();
        shared_unrolled_lock_longer_critical_section.unlock();
    }

    state.SetItemsProcessed(state.iterations());
}

static SpinLock shared_lock_longer_critical_section;
static std::uint64_t shared_loop_counter_longer_critical_section = 0;

static void BM_SpinLock_Loop_Contention_longer_critical_section(benchmark::State& state)
{
    benchmark::DoNotOptimize(&shared_loop_counter_longer_critical_section);

    for (auto _ : state)
    {
        shared_lock_longer_critical_section.lock_not_unrolled();

        for (int i = 0; i < 30; ++i)
        {
            _mm_pause();
            ++shared_loop_counter_longer_critical_section;
        }

        benchmark::ClobberMemory();
        shared_lock_longer_critical_section.unlock();
    }

    state.SetItemsProcessed(state.iterations());
}

BENCHMARK(BM_SpinLock_Unrolled_Contention)
    ->Threads(2)
    ->Threads(4)
    ->Threads(8)
    ->Threads(12)
    ->Threads(16)
    ->Threads(20);

BENCHMARK(BM_SpinLock_Loop_Contention)
    ->Threads(2)
    ->Threads(4)
    ->Threads(8)
    ->Threads(12)
    ->Threads(16)
    ->Threads(20);

BENCHMARK(BM_SpinLock_Unrolled_Contention_longer_critical_section)
    ->Threads(2)
    ->Threads(4)
    ->Threads(8)
    ->Threads(12)
    ->Threads(16)
    ->Threads(20);

BENCHMARK(BM_SpinLock_Loop_Contention_longer_critical_section)
    ->Threads(2)
    ->Threads(4)
    ->Threads(8)
    ->Threads(12)
    ->Threads(16)
    ->Threads(20);

BENCHMARK_MAIN();