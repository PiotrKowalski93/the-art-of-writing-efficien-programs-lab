#include <unistd.h>
#include <mutex>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include <atomic>

#include "benchmark/benchmark.h"

#define REPEAT2(x) x x
#define REPEAT4(x) REPEAT2(x) REPEAT2(x)
#define REPEAT8(x) REPEAT4(x) REPEAT4(x)
#define REPEAT16(x) REPEAT8(x) REPEAT8(x)
#define REPEAT32(x) REPEAT16(x) REPEAT16(x)
#define REPEAT(x) REPEAT32(x)

unsigned long x {0};
void BM_simple_increment(benchmark::State& state){
    for(auto _ : state){
        REPEAT(benchmark::DoNotOptimize(++x););
    }
}

std::atomic<unsigned long> x1 {0};
void BM_atomic_increment(benchmark::State& state){
    for(auto _ : state){
        REPEAT(benchmark::DoNotOptimize(++x1););
    }
}

unsigned long x2 {0};
void BM_mutex_increment(benchmark::State& state){
    std::mutex mtx;
    for(auto _ : state){
        REPEAT(
            {
                std::lock_guard<std::mutex> g(mtx);
                benchmark::DoNotOptimize(++x2);
            }
        );
    }
}

std::atomic<unsigned long> sum(0);
unsigned long sum_array[12*256];
void BM_shared(benchmark::State& state){
    unsigned long& local_x = sum_array[12*state.thread_index()];
    const size_t N = state.range(0);

    for(auto _ : state){
        for (size_t i = 0; i < N; i += 32) {
            REPEAT(benchmark::DoNotOptimize(++local_x););
        }
        sum += local_x;
    }

    state.SetItemsProcessed(N*state.iterations());
}

std::atomic<unsigned long> sum1(0);
unsigned long sum_array1[12];
void BM_false_shared(benchmark::State& state){
    unsigned long& local_x = sum_array1[state.thread_index()];
    const size_t N = state.range(0);

    for(auto _ : state){
        for (size_t i = 0; i < N; i += 32) {
            REPEAT(benchmark::DoNotOptimize(++local_x););
        }
        sum1 += local_x;
    }

    state.SetItemsProcessed(N*state.iterations());
}

std::atomic<unsigned long> x3(0);
void BM_local(benchmark::State& state) {
    unsigned long x = 0;
    const size_t N = state.range(0);
    for (auto _ : state) {
        for (size_t i = 0; i < N; i += 32) {
            REPEAT(benchmark::DoNotOptimize(++x););
        }
        x3 += x;
    }
    state.SetItemsProcessed(N*state.iterations());
}

static const long numcpu = sysconf(_SC_NPROCESSORS_CONF);

// BENCHMARK(BM_simple_increment)->Arg(1024)->ThreadRange(1, numcpu);
// BENCHMARK(BM_atomic_increment)->Arg(1024)->ThreadRange(1, numcpu);
// BENCHMARK(BM_mutex_increment)->Arg(1024)->ThreadRange(1, numcpu);

BENCHMARK(BM_shared)->Arg(1024)->ThreadRange(1, numcpu);
BENCHMARK(BM_false_shared)->Arg(1024)->ThreadRange(1, numcpu);
BENCHMARK(BM_local)->Arg(1024)->ThreadRange(1, numcpu);

BENCHMARK_MAIN();