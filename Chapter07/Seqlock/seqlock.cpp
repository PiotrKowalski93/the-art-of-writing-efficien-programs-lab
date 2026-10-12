#include <cstdint>
#include <atomic>
#include <barrier>
#include <thread>
#include <mutex>
#include <benchmark/benchmark.h>

struct MarketDataSnapshot {
    uint64_t sequence;
    int64_t timestamp;
    int64_t bid_price;
    int64_t ask_price;
    uint32_t bid_quantity;
    uint32_t ask_quantity;
};

static const struct timespec spin_wait_short = { 0, 1 };
static const struct timespec spin_wait_long  = { 0, 10000001 };
static inline void spin_wait_short_sleep() { nanosleep(&spin_wait_short, nullptr); }
static inline void spin_wait_long_sleep()  { nanosleep(&spin_wait_long,  nullptr); }

class SpinLock {
public:
    void lock() 
    {
       for(int spin_count = 0; lock_.load(std::memory_order_relaxed) || lock_.exchange(1, std::memory_order_acquire); ++spin_count)
        {
            if(!(lock_.load(std::memory_order_relaxed) || lock_.exchange(1, std::memory_order_acquire))) return;
            if(!(lock_.load(std::memory_order_relaxed) || lock_.exchange(1, std::memory_order_acquire))) return;
            if(!(lock_.load(std::memory_order_relaxed) || lock_.exchange(1, std::memory_order_acquire))) return;
            if(!(lock_.load(std::memory_order_relaxed) || lock_.exchange(1, std::memory_order_acquire))) return;
            if(!(lock_.load(std::memory_order_relaxed) || lock_.exchange(1, std::memory_order_acquire))) return;
            if(!(lock_.load(std::memory_order_relaxed) || lock_.exchange(1, std::memory_order_acquire))) return;
            if(!(lock_.load(std::memory_order_relaxed) || lock_.exchange(1, std::memory_order_acquire))) return;
            if(!(lock_.load(std::memory_order_relaxed) || lock_.exchange(1, std::memory_order_acquire))) return;

            if(spin_count < 8) {
                spin_wait_short_sleep();
            } else {
                spin_count = 0;
                spin_wait_long_sleep();
            }
        }
    };

    void unlock()
    {
        lock_.store(0, std::memory_order_release); 
    }

    bool locked() const
    {
        return lock_.load(std::memory_order_relaxed) == 1;
    }

private:
    std::atomic<int> lock_{0};
};

template <typename T>
class SequentialMutexLock {
public:
    void write(const T& data){
        std::lock_guard g(lock_);
        data_ = data;
    }

    T read_data() {
        std::lock_guard g(lock_);
        return data_;
    }
private:
    std::mutex lock_;
    T data_;
};

template <typename T>
class SpinLockWrapper {
public:
    void write(const T& data){
        std::lock_guard g(lock_);
        data_ = data;
    }

    T read_data() {
        std::lock_guard g(lock_);
        return data_;
    }
private:
    SpinLock lock_;
    T data_;
};

template <typename T>
class SequentialLock{
public: 
    void write(const T& data){
        while(true) {
            uint64_t cur = version_.load(std::memory_order::relaxed);

            // Another writer holds the lock
            if (cur % 2 != 0) continue;

            if (version_.compare_exchange_weak(cur, cur + 1, std::memory_order_acquire, std::memory_order_relaxed)) {
                data_.store(data, std::memory_order_release);
                version_.store(cur + 2, std::memory_order_release);
                return;
            }
        }
    }

    T read_data() {
        while(true) {
            uint64_t begin = version_.load(std::memory_order::relaxed);
            
            if (begin % 2 != 0) continue;

            T data = data_.load(std::memory_order_acquire);
            uint64_t end = version_.load(std::memory_order::relaxed);
            
            // We chceck if during read there is no change in data
            if(begin == end) return data;
        }
    }
private:
    std::atomic<uint64_t> version_;
    std::atomic<T> data_;
};

template <typename T>
class SequentialLockOneWriter{
public: 
    void write(const T& data){
        uint64_t cur = version_.load(std::memory_order_relaxed);

        version_.store(cur + 1, std::memory_order_release);
        data_.store(data, std::memory_order_relaxed);
        version_.store(cur + 2, std::memory_order_release);
    }

    T read_data() {
        while(true) {
            uint64_t begin = version_.load(std::memory_order::relaxed);
            
            if (begin % 2 != 0) continue;

            T data = data_.load(std::memory_order_acquire);
            uint64_t end = version_.load(std::memory_order::relaxed);
            
            if(begin == end) return data;
        }
    }
private:
    std::atomic<uint64_t> version_;
    std::atomic<T> data_;
};

// ============================================================================
// Baseline - mutex
// ============================================================================
// 1        Writers
// 2/4/6/8  Readers
template <typename LockType>
static void BM_MarkedDataAccess(benchmark::State& state){

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
BENCHMARK_TEMPLATE(BM_MarkedDataAccess, SequentialMutexLock<int64_t>)
    ->ArgsProduct({
        benchmark::CreateRange(2, 10, 2),
        {1},
    })
    ->UseRealTime();

BENCHMARK_TEMPLATE(BM_MarkedDataAccess, SequentialLock<int64_t>)
    ->ArgsProduct({
        benchmark::CreateRange(2, 10, 2),
        {1},
    })
    ->UseRealTime();

BENCHMARK_TEMPLATE(BM_MarkedDataAccess, SpinLockWrapper<int64_t>)
    ->ArgsProduct({
        benchmark::CreateRange(2, 10, 2),
        {1},
    })
    ->UseRealTime();

BENCHMARK_TEMPLATE(BM_MarkedDataAccess, SequentialLockOneWriter<int64_t>)
    ->ArgsProduct({
        benchmark::CreateRange(2, 10, 2),
        {1},
    })
    ->UseRealTime();

BENCHMARK_MAIN();