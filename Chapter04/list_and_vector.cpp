#include <algorithm>
#include <iostream>
#include <list>
#include <vector>

#include "benchmark/benchmark.h"

#define REPEAT2(x) x x
#define REPEAT4(x) REPEAT2(x) REPEAT2(x)
#define REPEAT8(x) REPEAT4(x) REPEAT4(x)
#define REPEAT16(x) REPEAT8(x) REPEAT8(x)
#define REPEAT32(x) REPEAT16(x) REPEAT16(x)
#define REPEAT(x) REPEAT32(x)

template <class Word>
void BM_write_list(benchmark::State& state){
    const size_t size = state.range(0);
    std::list<Word> c(size);
    Word x = {};

    for (auto _ : state)
    {
        for (auto it = c.begin(); it != c.end();)
        {
            REPEAT(benchmark::DoNotOptimize(*it++ = x);)
        }
        benchmark::ClobberMemory();
    }
}

template <class Word>
void BM_write_vector(benchmark::State& state){
    const size_t size = state.range(0);
    std::vector<Word> c(size);
    Word x = {};

    for (auto _ : state)
    {
        for (auto it = c.begin(), it_end = c.end(); it != it_end;)
        {
            REPEAT(benchmark::DoNotOptimize(*it++ = x);)
        }
        benchmark::ClobberMemory();
    }
}

// 1 048 576 = 1MB
BENCHMARK_TEMPLATE1(BM_write_list, unsigned long)->Arg(1<<20);
BENCHMARK_TEMPLATE1(BM_write_vector, unsigned long)->Arg(1<<20);

BENCHMARK_MAIN();