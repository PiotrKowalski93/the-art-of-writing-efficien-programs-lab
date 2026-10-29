#include <benchmark/benchmark.h>
#include <cstddef>
#include <new>
#include <vector>

#include "ringqueue.hpp"

// ------------------------------------------------------------------
// Benchmarks
// ------------------------------------------------------------------

void Benchmark_Throughput_SpinLock(benchmark::State& state) {
    static RingQueue<int*> *queue{};

    // We need power of 2
    const size_t queue_size = state.range(0);
    const size_t size_mask = queue_size - 1;
    const size_t bytes = queue_size * RingQueue<int*>::element_size();

    // First thread inits queue
    if(state.thread_index() == 0) {
        void* memory = ::operator new(bytes, std::align_val_t{RingQueue<int*>::element_align()});
        queue = new RingQueue<int*>(memory, bytes);
    }

    // Vector with data to share
    std::vector<int> vec(queue_size);
    for(size_t i = 0; i < queue_size; ++i) vec[i] = i;

    // Even threads are producers / uneven consumers
    const bool producer = state.thread_index() & 1;

    size_t p = 0, c = 0;
    size_t cmiss = 0, pmiss = 0;

    for(auto _ : state){
        if(producer){
            if(queue->push(&vec[p & size_mask])) ++p;
                else ++pmiss;
        }
        else {
            int* ptr = queue->pop();
            if(ptr) ++c;
                else ++cmiss;
            benchmark::DoNotOptimize(ptr);
        }
    }
};

BENCHMARK(Benchmark_Throughput_SpinLock)
    ->UseRealTime()
    ->ThreadRange(2, 32)
    ->Arg(1<<16);

BENCHMARK_MAIN();