#include <barrier>
#include <benchmark/benchmark.h>
#include <cstddef>
#include <memory>
#include <stop_token>
#include <thread>
#include <vector>

#include "ConcurrentAppendDeque.hpp"
#include "spinlock_deque.hpp"

// ------------------------------------------------------------------
// Benchmarks
// ------------------------------------------------------------------
// Determine the maximum number of hardware threads available on this system.
// This allows Google Benchmark to automatically scale the tests up to the
// machine's physical limits (e.g., all 256 threads on a Granite Rapids server)
// without hardcoding artificial ceilings.
// ============================================================================
// Benchmark Concurrency Design Principles
// ============================================================================
// These benchmarks do NOT use Google Benchmark's native MT (state.threads())
// nor do they create std::threads inside the timing loop. Why?
// 1. Thread Creation Overhead: Spawning threads takes significant OS time. If 
//    done inside the loop, the benchmark measures OS latency, not container speed.
// 2. Staggered Starts: Without precise synchronization, Thread 1 might finish its
//    work before Thread 8 even wakes up. This destroys concurrent contention.
//
// Clean Shutdown: While `std::jthread` is often used simply as an RAII wrapper 
// to avoid manual `.join()` calls, we actively use its cooperative cancellation 
// mechanism (`std::stop_token`). The main thread calls `.request_stop()` on all 
// threads and then unblocks the barrier one last time, allowing the workers to 
// check `stoken.stop_requested()` and cleanly exit their infinite loops.


// ============================================================================
// Scenario 1: Element Access with No Growth
// ============================================================================
// Goal: Measure pure, wait-free scaling when the deque is already sized.
// Expectation: Perfect linear scaling up to the memory bandwidth or ALU (Arthmetic 
// Logic Unit) limit, as threads write to strictly disjoint ranges without any 
// atomic synchronization.
template <typename DequeType>
static void BM_AccessNoGrowth(benchmark::State& state) {
    
    // Assigning parameters to vars
    size_t elements_per_thread = state.range(0);
    int num_threads = state.range(1);
    size_t total_elements = num_threads * elements_per_thread;

    // Pre-allocate the deque so no resizes occur during the benchmark.
    DequeType dq;
    dq.resize(total_elements);

    // num_threads workers + 1 main coordinator thread.
    std::barrier sync_start(num_threads + 1);
    std::barrier sync_end(num_threads + 1);

     // Using std::jthread automatically requests a stop and joins on destruction.
    std::vector<std::jthread> threads;
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&, t](std::stop_token stoken) {
            size_t start_idx = t * elements_per_thread;
            size_t end_idx = start_idx + elements_per_thread;
            
            while (true) {
                // Wait at the starting line for the main thread to resume timing.
                sync_start.arrive_and_wait();
                if (stoken.stop_requested()) break;
                
                // Payload: writes to a disjoint, per-thread index range, so no
                // two threads touch the same element. This is what exposes the
                // contrast between the two DequeType instantiations: for
                // ConcurrentAppendDeque each dq[i] is a wait-free acquire load of
                // the directory plus a plain store (no cross-thread coordination
                // on disjoint indices), whereas SpinlockDeque takes and releases
                // the global lock on every single element access -- the source of
                // its contention collapse.
                for (size_t i = start_idx; i < end_idx; ++i) {
                    dq[i] = static_cast<int>(i);
                }
                
                // Wait for all threads to finish before the next iteration.
                sync_end.arrive_and_wait();
            }
        });
    }
    
    for (auto _ : state) {
        sync_start.arrive_and_wait(); // Unleash threads, barier from coordinator thread
        sync_end.arrive_and_wait();   // Wait for completion, barier from coordinator thread
    }
    
    // Cleanly shut down the thread pool
    for (auto& thread : threads) {
        thread.request_stop();
    }

    // Unblock them one last time so they see the stop request
    sync_start.arrive_and_wait(); 
    threads.clear();
    
    // Custom counter for easy-to-read throughput (e.g., 20 G/s)
    state.counters["Elements/s"] = benchmark::Counter(state.iterations() * total_elements, benchmark::Counter::kIsRate);
}

// ============================================================================
// Scenario 2: Element Access with Size Check
// ============================================================================
// Goal: Measure the overhead of the `memory_order_acquire` penalty.
// In reality, a reader thread must check `size()` before blindly indexing. 
// This benchmark forces every thread to bounce the atomic `size_` cache line 
// once per iteration before writing to its disjoint range.
template <typename DequeType>
static void BM_AccessNoGrowthWithSizeCheck(benchmark::State& state) {
    size_t elements_per_thread = state.range(0);
    int num_threads = state.range(1);
    size_t total_elements = num_threads * elements_per_thread;

    DequeType dq;
    dq.resize(total_elements);

    std::barrier sync_start(num_threads + 1);
    std::barrier sync_end(num_threads + 1);

    std::vector<std::jthread> threads;
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&, t](std::stop_token stoken) {
            size_t start_idx = t * elements_per_thread;
            size_t end_idx = start_idx + elements_per_thread;
            
            while (true) {
                sync_start.arrive_and_wait();
                if (stoken.stop_requested()) break;
                
                if(dq.size() >= end_idx){
                    for (size_t i = start_idx; i < end_idx; ++i) {
                        dq[i] = static_cast<int>(i);
                    }
                }
                
                sync_end.arrive_and_wait();
            }
        });
    }
    
    for (auto _ : state) {
        sync_start.arrive_and_wait();
        sync_end.arrive_and_wait();
    }
    
    for (auto& thread : threads) {
        thread.request_stop();
    }

    sync_start.arrive_and_wait(); 
    threads.clear();

    state.counters["Elements/s"] = benchmark::Counter(state.iterations() * total_elements, benchmark::Counter::kIsRate);
}

// ============================================================================
// Scenario 3: Resize and Write
// ============================================================================
// Goal: Stress test the Double-Checked Locking Pattern (DCLP) and lock-free
// contention during container expansion.
// The deque starts empty. All threads simultaneously try to resize it to their
// required capacity, guaranteeing a massive "Thundering Herd" collision.
template <typename DequeType>
static void BM_ResizeAndWrite(benchmark::State& state){
    size_t elements_per_thread = state.range(0);
    int num_threads = state.range(1);
    
    // Benchmark Balance Decision:
    // If we only write each element once, the extremely slow heap allocations
    // and spinlock contention inside `resize()` will completely dominate the time,
    // masking the wait-free read/write throughput we actually care about.
    // Real-world usage involves resizing rarely, and accessing data heavily.
    // Therefore, we amplify the access workload by 100x to simulate this reality.
    const int work_iterations = 100;
    size_t total_elements = num_threads * elements_per_thread * work_iterations;
    
    std::unique_ptr<DequeType> dq;
    
    std::barrier sync_start(num_threads + 1);
    std::barrier sync_end(num_threads + 1);
    
    std::vector<std::jthread> threads;
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&, t](std::stop_token stoken) {
            size_t start_idx = t * elements_per_thread;
            size_t end_idx = start_idx + elements_per_thread;
            
            while (true) {
                sync_start.arrive_and_wait();
                if (stoken.stop_requested()) break;
                
                // The Thundering Herd hits the DCLP here. Only one thread
                // will successfully grab the lock and allocate the blocks.
                dq->resize(end_idx);
                
                // The amplified read/write workload. 
                // We add `iter` to prevent compiler loop-unrolling optimizations.
                for (int iter = 0; iter < work_iterations; ++iter) {
                    for (size_t i = start_idx; i < end_idx; ++i) {
                        (*dq)[i] = static_cast<int>(i + iter);
                    }
                }
                
                sync_end.arrive_and_wait();
            }
        });
    }
    
    for (auto _ : state) {
        // Pause the timer so we do not benchmark the teardown of the old
        // deque or the instantiation of the new one. We strictly only want
        // to measure the concurrent scaling.
        state.PauseTiming();
        dq = std::make_unique<DequeType>();
        state.ResumeTiming();
        
        sync_start.arrive_and_wait(); // Unleash threads
        sync_end.arrive_and_wait();   // Wait for completion
    }
    
    for (auto& thread : threads) {
        thread.request_stop();
    }
    sync_start.arrive_and_wait();
    threads.clear();
    
    state.counters["Elements/s"] = benchmark::Counter(
        state.iterations() * total_elements,
        benchmark::Counter::kIsRate);
}

// ============================================================================
// Scenario 1: Element Access with No Growth
// ============================================================================
BENCHMARK_TEMPLATE(BM_AccessNoGrowth, SpinlockDeque<int>)
    ->ArgsProduct({
        {100000},
        benchmark::CreateRange(1, 32, 2)
    })->UseRealTime();

BENCHMARK_TEMPLATE(BM_AccessNoGrowth, ConcurrentAppendDeque<int, 1024>)
    ->ArgsProduct({
        {100000},
        benchmark::CreateRange(1, 32, 2)
    })->UseRealTime();

// ============================================================================
// Scenario 2: Element Access with Size Check
// ============================================================================
BENCHMARK_TEMPLATE(BM_AccessNoGrowthWithSizeCheck, SpinlockDeque<int>)
    ->ArgsProduct({
        {100000},
        benchmark::CreateRange(1, 32, 2)
    })->UseRealTime();

BENCHMARK_TEMPLATE(BM_AccessNoGrowthWithSizeCheck, ConcurrentAppendDeque<int, 1024>)
    ->ArgsProduct({
        {100000},
        benchmark::CreateRange(1, 32, 2)
    })->UseRealTime();

// ============================================================================
// Scenario 3: Resize and Write
// ============================================================================
BENCHMARK_TEMPLATE(BM_ResizeAndWrite, SpinlockDeque<int>)
    ->ArgsProduct({
        {100000},
        benchmark::CreateRange(1, 32, 2)
    })->UseRealTime();

BENCHMARK_TEMPLATE(BM_ResizeAndWrite, ConcurrentAppendDeque<int, 1024>)
    ->ArgsProduct({
        {100000},
        benchmark::CreateRange(1, 32, 2)
    })->UseRealTime();


BENCHMARK_MAIN();