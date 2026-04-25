#include <cstddef>
#include <cstdlib>
#include <string.h>
#include <emmintrin.h>
#include <immintrin.h>
#include <vector>
#include <algorithm>

#include "benchmark/benchmark.h"

#define REPEAT2(x) x x
#define REPEAT4(x) REPEAT2(x) REPEAT2(x)
#define REPEAT8(x) REPEAT4(x) REPEAT4(x)
#define REPEAT16(x) REPEAT8(x) REPEAT8(x)
#define REPEAT32(x) REPEAT16(x) REPEAT16(x)
#define REPEAT(x) REPEAT32(x)

template <class Word>
void BM_sequential_read(benchmark::State& state){
    const size_t size = state.range(0);

    void* memory = ::malloc(size);
    void* const end = static_cast<char*>(memory) + size;

    volatile Word* const p0 = static_cast<Word*>(memory);
    Word* const p1 = static_cast<Word*>(end);

    for (auto _ : state)
    {
        for (volatile Word* p = p0; p != p1;)
        {
            // Loop unrolling to measue memory not CPU cycles stalls
            REPEAT(benchmark::DoNotOptimize(*p++);)
        }
        benchmark::ClobberMemory();
    }
    
    ::free(memory);

    state.SetBytesProcessed(size*state.iterations());
    state.SetItemsProcessed((p1 - p0)*state.iterations());
}

template <class Word>
void BM_sequential_write(benchmark::State& state){
    const size_t size = state.range(0);

    void* memory = ::malloc(size);
    void* const end = static_cast<char*>(memory) + size;

    volatile Word* const p0 = static_cast<Word*>(memory);
    Word* const p1 = static_cast<Word*>(end);

    Word fill = {};

    for (auto _ : state)
    {
        for (volatile Word* p = p0; p != p1;)
        {
            // Loop unrolling to measue memory not CPU cycles stalls
            REPEAT(benchmark::DoNotOptimize(*p++ = fill);)
        }
        benchmark::ClobberMemory();
    }
    
    ::free(memory);

    state.SetBytesProcessed(size*state.iterations());
    state.SetItemsProcessed((p1 - p0)*state.iterations());
}

// 1<<10 = 1024 (1KB)
// 1<<30 = 1 073 741 824 (1GB)
// We run benchmark dubeling word size to read each run from 1KB to 1GB
#define ARGS ->RangeMultiplier(2)->Range(1<<10, 1<<30)

BENCHMARK_TEMPLATE1(BM_sequential_read, unsigned int) ARGS;
BENCHMARK_TEMPLATE1(BM_sequential_read, unsigned long) ARGS;

BENCHMARK_TEMPLATE1(BM_sequential_write, unsigned int) ARGS;
BENCHMARK_TEMPLATE1(BM_sequential_write, unsigned long) ARGS;

// BENCHMARK_TEMPLATE1(BM_sequential_read, uint8_t) ARGS;
// BENCHMARK_TEMPLATE1(BM_sequential_read, uint16_t) ARGS;
// BENCHMARK_TEMPLATE1(BM_sequential_read, uint32_t) ARGS;
// BENCHMARK_TEMPLATE1(BM_sequential_read, uint64_t) ARGS;

BENCHMARK_MAIN();