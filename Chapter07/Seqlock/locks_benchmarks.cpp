#include <benchmark/benchmark.h>
#include <barrier>
#include <cstdint>
#include <memory>
#include <thread>
#include <vector>

#include "../MutexLock/mutexlock.hpp"
#include "../SpinLock/spinlockwrapper.hpp"
#include "seqlock.hpp"
#include "seqlock_onewriter.hpp"

template <typename LockType>
static void BM_Locks(benchmark::State& state){

    uint8_t num_reader_threads = state.range(0);
    uint8_t num_writer_threads = state.range(1);
    uint8_t main_sync_thread = 1;

    uint16_t number_of_reads_per_reader = 10000;
    uint16_t number_of_writes_per_writer = 3000;

    std::barrier sync_start(num_reader_threads + num_writer_threads + main_sync_thread);
    std::barrier sync_end(num_reader_threads + num_writer_threads + main_sync_thread);

    std::vector<std::jthread> writer_threads;
    std::vector<std::jthread> reader_threads;

    std::unique_ptr<LockType> lock_ = std::make_unique<LockType>();

    //static const MarketDataSnapshot data {1, 2, 234, 234, 1000, 1000 };
    static const int64_t var = 2345;

    // Starting Readers
    for (uint16_t t = 0; t < num_reader_threads; ++t) {
        reader_threads.emplace_back([&, t](std::stop_token stoken) {
            while (true) {
                sync_start.arrive_and_wait();
                if (stoken.stop_requested()) break;
                
                for (int16_t i = 0; i < number_of_reads_per_reader; ++i) {
                    auto x = lock_->read_data();
                    benchmark::DoNotOptimize(x);
                }
                
                sync_end.arrive_and_wait();
            }
        });
    }
    
    // Starting Writers
    for (uint16_t t = 0; t < num_writer_threads; ++t) {
         writer_threads.emplace_back([&, t](std::stop_token stoken) {
            while (true) {
                sync_start.arrive_and_wait();
                if (stoken.stop_requested()) break;
                
                for (int16_t i = 0; i < number_of_writes_per_writer; ++i) {
                    lock_->write(var);
                }
                
                sync_end.arrive_and_wait();
            }
        });
    }

    for (auto _ : state) {
        // Pause the timer so we do not benchmark the teardown of the old
        // deque or the instantiation of the new one. We strictly only want
        // to measure the concurrent scaling.
        // state.PauseTiming();
        // lock_ = std::make_unique<SequentialMutexLock<MarketDataSnapshot>>();
        // state.ResumeTiming();
        
        sync_start.arrive_and_wait(); // Unleash threads
        sync_end.arrive_and_wait();   // Wait for completion
    }

    for (auto& thread : writer_threads) {
        thread.request_stop();
    }

    for (auto& thread : reader_threads) {
        thread.request_stop();
    }

    sync_start.arrive_and_wait();
    writer_threads.clear();
    reader_threads.clear();

    state.SetItemsProcessed(state.iterations() * (num_reader_threads * number_of_reads_per_reader + num_writer_threads * number_of_writes_per_writer));
};

// ============================================================================
// Baseline
// ============================================================================
BENCHMARK_TEMPLATE(BM_Locks, MutexLock<int64_t>)
    ->ArgsProduct({
        benchmark::CreateRange(2, 10, 2),
        {1},
    })
    ->UseRealTime();

BENCHMARK_TEMPLATE(BM_Locks, SequentialLock<int64_t>)
    ->ArgsProduct({
        benchmark::CreateRange(2, 10, 2),
        {1},
    })
    ->UseRealTime();

BENCHMARK_TEMPLATE(BM_Locks, SpinLockWrapper<int64_t>)
    ->ArgsProduct({
        benchmark::CreateRange(2, 10, 2),
        {1},
    })
    ->UseRealTime();

BENCHMARK_TEMPLATE(BM_Locks, SequentialLockOneWriter<int64_t>)
    ->ArgsProduct({
        benchmark::CreateRange(2, 10, 2),
        {1},
    })
    ->UseRealTime();

BENCHMARK_MAIN();