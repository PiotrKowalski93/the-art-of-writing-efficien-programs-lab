#include <unistd.h>
#include <mutex>

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

static const long numcpu = sysconf(_SC_NPROCESSORS_CONF);

BENCHMARK(BM_simple_increment)->Arg(1024)->ThreadRange(1, numcpu);
BENCHMARK(BM_atomic_increment)->Arg(1024)->ThreadRange(1, numcpu);
BENCHMARK(BM_mutex_increment)->Arg(1024)->ThreadRange(1, numcpu);

BENCHMARK_MAIN();