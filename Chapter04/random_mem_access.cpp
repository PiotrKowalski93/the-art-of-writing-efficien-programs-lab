#include <cstddef>
#include <cstdlib>
#include <string.h>
#include <emmintrin.h>
#include <immintrin.h>
#include <vector>
#include <algorithm>
#include <random>

#include "benchmark/benchmark.h"

#define REPEAT2(x) x x
#define REPEAT4(x) REPEAT2(x) REPEAT2(x)
#define REPEAT8(x) REPEAT4(x) REPEAT4(x)
#define REPEAT16(x) REPEAT8(x) REPEAT8(x)
#define REPEAT32(x) REPEAT16(x) REPEAT16(x)
#define REPEAT(x) REPEAT32(x)

template <class Word>
void BM_random_read(benchmark::State& state){
const size_t size = state.range(0);

    // -------- Memory allocation -------
    // -------- start and end ptrs ------
    void* memory = ::malloc(size);
    void* const end = static_cast<char*>(memory) + size;

    volatile Word* const p0 = static_cast<Word*>(memory);
    Word* const p1 = static_cast<Word*>(end);
    // ----------------------------------

    // ------ Random Addresses ----------
    const size_t N = size/sizeof(Word);
    std::vector<int> v_index(N);
    for (size_t i = 0; i < N; ++i)
    {
        v_index[i] = i;
    }
    auto rng = std::default_random_engine {};
    std::shuffle(v_index.begin(), v_index.end(), rng);

    int* const index = v_index.data();
    int* const i1 = index + N;
    // ----------------------------------

    Word fill;
    memset(&fill, 0x0f, sizeof(fill));

    for (auto _ : state)
    {
        for (const int* ind = index; ind < i1;)
        {
            REPEAT(benchmark::DoNotOptimize(*(p0 + *ind++));)
        }
        
        benchmark::ClobberMemory();
    }
    
    ::free(memory);

    state.SetBytesProcessed(size*state.iterations());
    state.SetItemsProcessed((p1 - p0)*state.iterations());
}

template <class Word>
void BM_random_write(benchmark::State& state){
    const size_t size = state.range(0);

    // -------- Memory allocation -------
    // -------- start and end ptrs ------
    void* memory = ::malloc(size);
    void* const end = static_cast<char*>(memory) + size;

    volatile Word* const p0 = static_cast<Word*>(memory);
    Word* const p1 = static_cast<Word*>(end);
    // ----------------------------------

    // ------ Random Addresses ----------
    const size_t N = size/sizeof(Word);
    std::vector<int> v_index(N);
    for (size_t i = 0; i < N; ++i)
    {
        v_index[i] = i;
    }
    auto rng = std::default_random_engine {};
    std::shuffle(v_index.begin(), v_index.end(), rng);

    int* const index = v_index.data();
    int* const i1 = index + N;
    // ----------------------------------

    Word fill;
    memset(&fill, 0x0f, sizeof(fill));

    for (auto _ : state)
    {
        for (const int* ind = index; ind < i1;)
        {
            REPEAT(*(p0 + *ind++) = fill;)
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

BENCHMARK_TEMPLATE1(BM_random_read, unsigned int) ARGS;
BENCHMARK_TEMPLATE1(BM_random_read, unsigned long) ARGS;

BENCHMARK_TEMPLATE1(BM_random_write, unsigned int) ARGS;
BENCHMARK_TEMPLATE1(BM_random_write, unsigned long) ARGS;

// BENCHMARK_TEMPLATE1(BM_random_read, uint8_t) ARGS;
// BENCHMARK_TEMPLATE1(BM_random_read, uint16_t) ARGS;
// BENCHMARK_TEMPLATE1(BM_random_read, uint32_t) ARGS;
// BENCHMARK_TEMPLATE1(BM_random_read, uint64_t) ARGS;

BENCHMARK_MAIN();