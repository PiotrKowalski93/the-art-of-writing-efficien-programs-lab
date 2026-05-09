#include <unistd.h>
#include <mutex>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <atomic>

#include "benchmark/benchmark.h"

std::mutex m;
size_t count = 0;
void BM_mutex(benchmark::State& state){
    for(auto _ : state){
        std::lock_guard lock_(m);
        ++count;
    }
    state.SetItemsProcessed(state.iterations());
}

std::atomic<size_t> count_seq = 0;
void BM_atomic_seq_cast(benchmark::State& state){
    for(auto _ : state){
        ++count_seq;
    }
    state.SetItemsProcessed(state.iterations());
}

std::atomic<size_t> count_rlx = 0;
void BM_atomic_relaxed(benchmark::State& state){
    for(auto _ : state){
        count_rlx.fetch_add(1, std::memory_order_relaxed);
    }
    state.SetItemsProcessed(state.iterations());
}

std::atomic<size_t> count_cas_strong = 0;
void BM_CAS_strong(benchmark::State& state){
    //size_t local_retries = 0;

    for(auto _ : state){
        size_t expected = count_cas_strong.load(std::memory_order_relaxed);

        while (!count_cas_strong.compare_exchange_strong(
            expected,
            expected + 1,
            std::memory_order_relaxed,
            std::memory_order_relaxed))
        {
            //++local_retries;
        }
    }
    state.SetItemsProcessed(state.iterations());
    //state.counters["per_thread_retries"] = local_retries;
}

std::atomic<size_t> count_cas_weak = 0;
void BM_CAS_weak(benchmark::State& state){
    //size_t local_retries = 0;

    for(auto _ : state){
        size_t expected = count_cas_weak.load(std::memory_order_relaxed);

        while (!count_cas_weak.compare_exchange_weak(
            expected,
            expected + 1,
            std::memory_order_relaxed,
            std::memory_order_relaxed))
        {
            //++local_retries;
        }
    }
    state.SetItemsProcessed(state.iterations());
    //state.counters["per_thread_retries"] = local_retries;
}


class Spinlock{
    public:
        void lock(){
            for (int i = 0; flag_.load(std::memory_order_relaxed) || flag_.exchange(1, std::memory_order_acquire); ++i)
            {
                if(i == 8){
                    lock_sleep();
                    i = 0;
                }
            }
        }

        void unlock(){
            flag_.exchange(0, std::memory_order_release);
        }

    private:
        std::atomic<unsigned int> flag_;

        void lock_sleep(){
            static const timespec ns = {0, 1,};
            nanosleep(&ns, NULL);
        }
};

Spinlock spinlock;
size_t count_x = 0;

void BM_spinlock(benchmark::State& state) {
    if (state.thread_index() == 0) {
        count_x = 0;
    }

    for (auto _ : state) {
        std::lock_guard<Spinlock> L(spinlock);
        benchmark::DoNotOptimize(++count_x);
        // unlock on destructor
    }

    state.SetItemsProcessed(state.iterations());
}

// void BM_spinlock(benchmark::State& state) {
//     if (state.thread_index() == 0) {
//         count_x = 0;
//     }

//     for (auto _ : state) {
//         std::lock_guard<Spinlock> L(spinlock);
//         benchmark::DoNotOptimize(++count_x);
//         // unlock on destructor
//     }

//     state.SetItemsProcessed(state.iterations());
// }


BENCHMARK(BM_mutex)->Threads(1) \
    ->Threads(2)
    ->Threads(4)
    ->Threads(8)
    ->Threads(16)
    ->UseRealTime();

BENCHMARK(BM_atomic_seq_cast)->Threads(1) \
    ->Threads(2)
    ->Threads(4)
    ->Threads(8)
    ->Threads(16)
    ->UseRealTime();

BENCHMARK(BM_atomic_relaxed)->Threads(1) \
    ->Threads(2)
    ->Threads(4)
    ->Threads(8)
    ->Threads(16)
    ->UseRealTime();

BENCHMARK(BM_CAS_strong)->Threads(1) \
    ->Threads(2)
    ->Threads(4)
    ->Threads(8)
    ->Threads(16)
    ->UseRealTime();

BENCHMARK(BM_CAS_weak)->Threads(1) \
    ->Threads(2)
    ->Threads(4)
    ->Threads(8)
    ->Threads(16)
    ->UseRealTime();

BENCHMARK(BM_spinlock)->Threads(1) \
    ->Threads(2)
    ->Threads(4)
    ->Threads(8)
    ->Threads(16)
    ->UseRealTime();


BENCHMARK_MAIN();