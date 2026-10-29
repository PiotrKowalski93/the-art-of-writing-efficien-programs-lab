#include <benchmark/benchmark.h>

#include "readwrite_stack.hpp"

namespace {

readwrite_stack<int> stack;

void BM_readwrite_stack(benchmark::State& state) {
    const size_t count = state.range(0);
    for (auto _ : state) {
        for (size_t i = 0; i < count; ++i) {
            stack.push(static_cast<int>(i));
        }
        for (size_t i = 0; i < count; ++i) {
            benchmark::DoNotOptimize(stack.pop());
        }
    }
    state.SetItemsProcessed(state.iterations() * count);
}

void BM_readwrite_top(benchmark::State& state) {
    const size_t count = state.range(0);
    for (size_t i = 0; i < count; ++i) {
        stack.push(static_cast<int>(i));
    }

    for (auto _ : state) {
        benchmark::DoNotOptimize(stack.top());
        for (int i = 0; i < 1000; ++i) {
            for (int j = 0; j < 1000; ++j) {
                benchmark::DoNotOptimize(j);
            }
            benchmark::DoNotOptimize(i);
        }
    }
}

BENCHMARK(BM_readwrite_top)->Arg(64)->Threads(1)->Threads(2)->Threads(4)->Threads(8);
BENCHMARK(BM_readwrite_stack)->Arg(1024)->Threads(1)->Threads(2)->Threads(4)->Threads(8);

}

BENCHMARK_MAIN();
