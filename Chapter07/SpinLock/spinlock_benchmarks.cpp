#include <benchmark/benchmark.h>
#include <cstdint>
#include <immintrin.h>

#include "spinlock.hpp"

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