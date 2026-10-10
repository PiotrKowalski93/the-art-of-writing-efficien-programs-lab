#include <stddef.h>
#include <atomic>
#include <vector>
#include <time.h>
#include <unistd.h>
#include <deque>
#include <barrier>
#include <memory>
#include <cstdint>
#include <thread>
#include <benchmark/benchmark.h>

static const struct timespec spin_wait_short = { 0, 1 };
static const struct timespec spin_wait_long  = { 0, 10000001 };
static inline void spin_wait_short_sleep() { nanosleep(&spin_wait_short, nullptr); }
static inline void spin_wait_long_sleep()  { nanosleep(&spin_wait_long,  nullptr); }

// ------------------------------------------------------------------
// Spinlock for ConcurrentAppendDeque
// ------------------------------------------------------------------
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

// ------------------------------------------------------------------
// Interface
// ------------------------------------------------------------------
template <typename T, size_t BlockSize = 2048>
class ConcurrentAppendDeque {
    static constexpr size_t BlockMask = BlockSize - 1;
    static constexpr size_t BlockShift = std::countr_zero(BlockSize);

    static_assert(BlockSize > 0 && ((BlockSize & (BlockSize - 1)) == 0), "BlockSize must by power of 2");

public:
    ConcurrentAppendDeque() = default;
    ~ConcurrentAppendDeque();

    size_t size();
    
    T& operator[](size_t index);
    const T& operator[](size_t index) const;

    void reserve(size_t new_capacity);
    void resize(size_t new_size);
    void push_back(const T& value);

    template<typename... Args>
    void emplace_back(Args&&... args);

private:
    struct alignas(T) Block {
        unsigned char data[BlockSize * sizeof(T)];
    };
    
    void reallocate_dictionary(size_t new_capacity);
    T* allocate_block();
    std::atomic<T**> directory_;
    size_t capacity_ {};
    std::vector<T**> retired_directories_ {};
    std::atomic<size_t> size_ {};

    // We use mutable to be able to lock (modify) object state in const method
    mutable SpinLock spinlock_;

    
};

// ------------------------------------------------------------------
// Implementation
// ------------------------------------------------------------------
template <typename T, size_t BlockSize>
size_t ConcurrentAppendDeque<T, BlockSize>::size() {
    return size_.load(std::memory_order_acquire);
}

template <typename T, size_t BlockSize>
T* ConcurrentAppendDeque<T, BlockSize>::allocate_block(){
    Block* b = new Block{};
    return reinterpret_cast<T*>(b->data);
}

template <typename T, size_t BlockSize>
ConcurrentAppendDeque<T, BlockSize>::~ConcurrentAppendDeque() {
    size_t current_size = size_.load(std::memory_order_relaxed);
    T** dir = directory_.load(std::memory_order_relaxed);

    if(dir) {
        for(size_t i = 0; i < current_size; ++i){
            size_t block_idx = i >> BlockShift;
            size_t local_idx = i & BlockMask;
            dir[block_idx][local_idx].~T();
        }

        for(size_t i = 0; i < capacity_; ++i){
            if (dir[i]) {
                Block* b = reinterpret_cast<Block*>(dir[i]);
                delete b;
            }
        }

        delete[] dir;
    }

    for (T** retired : retired_directories_) {
        delete[] retired;
    }
}

template <typename T, size_t BlockSize>
T& ConcurrentAppendDeque<T, BlockSize>::operator[](size_t index){
    T** dir = directory_.load(std::memory_order_acquire);
    size_t block_idx = index >> BlockShift;
    size_t local_idx = index & BlockMask;
    return dir[block_idx][local_idx];
}

template <typename T, size_t BlockSize>
void ConcurrentAppendDeque<T, BlockSize>::resize(size_t new_size){
    size_t current_size = size_.load(std::memory_order_acquire);
    
    // We do not allow shrinking
    if(new_size <= current_size) return;

    // Critical section begins
    std::lock_guard lock(spinlock_);

    // We need to be sure that nothing changed
    current_size = size_.load(std::memory_order_acquire);

    // ------ Double-Check Locking Pattern ------
    // Other thread might resized que faster
    if(new_size > current_size){
        size_t new_capacity = (new_size + BlockMask) >> BlockShift;
        if(new_capacity > capacity_){
            reallocate_dictionary(new_capacity);
        }

        // Allocate new T** and move all pointers
        T** dir = directory_.load(std::memory_order_relaxed);
        for(size_t i = 0; i < current_size; ++i){
            size_t block_idx = i >> BlockShift;
            size_t offset = i & BlockMask;
            // Block is not allocated
            if(!dir[block_idx]){
                dir[block_idx] = allocate_block();
            }
            std::construct_at(&dir[block_idx][offset]);
        }
        size_.store(new_size, std::memory_order_release);
    }
}

template <typename T, size_t BlockSize>
void ConcurrentAppendDeque<T, BlockSize>::reallocate_dictionary(size_t new_capacity){
    T** new_dir = new T*[new_capacity];
    // We can use relaxed because we are locked
    T** old_dir = directory_.load(std::memory_order_relaxed);
    
    for(size_t i = 0; i < capacity_; ++i){
        // 'moving' pointers to new collection of blocks
        new_dir[i] = old_dir[i];
    }

    for(size_t i = capacity_; i < new_capacity; ++i){
        // Empty new spaces
        new_dir[i] = nullptr;
    }

    directory_.store(new_dir, std::memory_order_release);

    if(old_dir){
        retired_directories_.push_back(old_dir);
    }
    capacity_ = new_capacity;
}


// ------------------------------------------------------------------
// Benchmarks
// ------------------------------------------------------------------
// Determine the maximum number of hardware threads available on this system.
// This allows Google Benchmark to automatically scale the tests up to the
// machine's physical limits (e.g., all 256 threads on a Granite Rapids server)
// without hardcoding artificial ceilings.
static const int num_cpu = sysconf(_SC_NPROCESSORS_CONF);


// ============================================================================
// SpinlockDeque Baseline
// ============================================================================
// A simple thread-safe wrapper around std::deque using a SpinLock.
// This serves as the baseline to demonstrate the catastrophic performance
// collapse (thundering herd) that occurs when using locks under high contention,
// compared to the lock-free ConcurrentAppendDeque.
template <typename T>
class SpinlockDeque {
public:
    void push_back(const T& val) {
        std::lock_guard<SpinLock> lock(spinlock_);
        deque_.push_back(val);
    }
    
    void resize(size_t new_size) {
        std::lock_guard<SpinLock> lock(spinlock_);
        if (new_size > deque_.size()) {
            deque_.resize(new_size);
        }
    }

    size_t size() const {
        std::lock_guard<SpinLock> lock(spinlock_);
        return deque_.size();
    }
    
    T& operator[](size_t index) {
        std::lock_guard<SpinLock> lock(spinlock_);
        return deque_[index];
    }
    
    const T& operator[](size_t index) const {
        std::lock_guard<SpinLock> lock(spinlock_);
        return deque_[index];
    }
    
private:
    mutable SpinLock spinlock_;
    std::deque<T> deque_;
};

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

BENCHMARK_TEMPLATE(BM_AccessNoGrowth, SpinlockDeque<int>)
    ->ArgsProduct({
        {100000},
        benchmark::CreateRange(1, num_cpu, /*multi=*/2)
    })->UseRealTime();

BENCHMARK_TEMPLATE(BM_AccessNoGrowth, ConcurrentAppendDeque<int, 1024>)
    ->ArgsProduct({
        {100000},
        benchmark::CreateRange(1, num_cpu, /*multi=*/2)
    })->UseRealTime();







BENCHMARK_MAIN();