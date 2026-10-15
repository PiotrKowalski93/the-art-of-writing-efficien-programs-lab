#include <cstdint>
#include <atomic>
#include <barrier>
#include <thread>
#include <mutex>
#include <queue>
#include <optional>
#include <type_traits>
#include <utility>
#include <cstring>
#include <benchmark/benchmark.h>

static const struct timespec spin_wait_short = { 0, 1 };
static const struct timespec spin_wait_long  = { 0, 10000001 };
static inline void spin_wait_short_sleep() { nanosleep(&spin_wait_short, nullptr); }
static inline void spin_wait_long_sleep()  { nanosleep(&spin_wait_long,  nullptr); }

// ------------------------------------------------------------------
// Locks
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
class mt_queue {
    std::queue<T> queue_;
    mutable SpinLock lock_;
public: 
    void push(const T& data){
        std::lock_guard q(lock_);
        queue_.push(data);
    }

    std::optional<T> pop() {
        std::lock_guard g(lock_);

        if(queue_.empty()) {
            return std::nullopt;
        } 

        std::optional<T> result(std::move(queue_.front()));
        queue_.pop();

        return result;
    }
};

// ------------------------------------------------------------------
// Implementation
// ------------------------------------------------------------------
// TODO: Implement ctor and dtor
template <typename Key, typename Value = void, size_t NTRY = 8>  // Number of attempts to acquire a slot before giving up
class RingQueue {
public:
    //explicit RingQueue(size_t capacity);
    RingQueue(void* memory, size_t bytes) : 
        queue_(static_cast<slot_t*>(memory)),
        capacity_(std::bit_floor(bytes/sizeof(slot_t))),
        capacity_mask_(capacity_ - 1)
    {
        for (size_t i = 0; i < capacity_; ++i) {
            std::construct_at(&queue_[i]);
            queue_[i].key.store(Key{}, std::memory_order_relaxed);
        }
    }

    ~RingQueue() { 
        for (size_t i = read_position_.i; i != write_position_.i; ++i) {
            slot_t& slot = queue_[i & capacity_mask_];
            if constexpr (!std::is_same_v<Value, void>) {
                slot.value.~Value();
            }
            slot.key.store(Key{}, std::memory_order_relaxed);
        }
    }

    template <typename V> requires std::is_constructible_v<Value, V>
    bool push(Key key, V&& value);
    bool push(Key key) requires std::is_same_v<Value, void>;
    template <typename V = Value> requires (!std::is_same_v<void, V>)
    Key pop(V& value);
    Key pop() requires std::is_same_v<Value, void>;
    
    static constexpr size_t element_size() { 
        return sizeof(slot_t);  
    };

    size_t capacity() const {
        return capacity_;
    };

    static constexpr size_t element_align() { 
        return alignof(slot_t); 
    }

    bool empty() const {
        std::unique_lock l(write_position_.lock);
        slot_t& slot = queue_[write_position_.i & capacity_mask_];

        Key key = slot.load(std::memory_order_relaxed);
        return key == Key{};
    }

private:
    // General case with Key and Value
    template <typename K, typename V>
    struct alignas(256) queue_slot_t{
        std::atomic<K> key;
        std::atomic<int> busy_flag; // Guard for value contruction
        V value;
    };

    // For fast path we store only Key
    template <typename K>
    struct alignas(256) queue_slot_t<K, void> {
        std::atomic<K> key;
    };

    struct alignas(256) position_t {
        mutable SpinLock lock;
        size_t i {};
    };

    using slot_t = queue_slot_t<Key, Value>;

    // ------ Queue Data ------
    slot_t* const queue_;
    size_t const capacity_;
    size_t const capacity_mask_;

    // Producer
    position_t write_position_;

    // Consumer
    position_t read_position_;
};

template <typename Key, typename Value, size_t NTRY>
bool RingQueue<Key, Value, NTRY>::push(Key key) requires std::is_same_v<Value, void> {
    std::unique_lock<SpinLock> l(write_position_.lock);
    slot_t& slot = queue_[write_position_.i & capacity_mask_];

    for(size_t i = 0; i != NTRY; ++i){
        if(slot.key.load(std::memory_order_acquire) == Key{}){
            slot.key.store(key, std::memory_order_release);
            ++write_position_.i;
            return true;
        }
    }

    return false;
};

template <typename Key, typename Value, size_t NTRY>
Key RingQueue<Key, Value, NTRY>::pop() requires std::is_same_v<Value, void> {
    std::unique_lock<SpinLock> l(read_position_.lock);
    slot_t& slot = queue_[read_position_.i & capacity_mask_];

    Key ret = slot.key.load(std::memory_order_acquire);
    if (ret == Key{}) return Key{};
    slot.key.store(Key{}, std::memory_order_release);

    ++read_position_.i;
    return ret;
};

template <typename Key, typename Value, size_t NTRY>
template <typename V> 
    requires std::is_constructible_v<Value, V>
bool RingQueue<Key, Value, NTRY>::push(Key key, V&& value) {
    std::unique_lock<SpinLock> l(write_position_.lock);
    slot_t& slot = queue_[write_position_.i & & capacity_mask_];

    for(size_t i = 0; i != NTRY; ++i){
        if(slot.key.load(std::memory_order_acquire) == Key{} &&
            !slot.busy_flag.load(std::memory_order_acquire) &&
            slot.key.load(std::memory_order_acquire) == Key{})
        {
            slot.busy_flag.store(1, std::memory_order_release);
            ++write_position_.i;
            l.unlock();

            ::new(&slot.value) Value(std::forward<V>(value));
            slot.key.store(key, std::memory_order_release);
            slot.busy_flag.store(0, std::memory_order_release);

            return true;
        }
    }
    return false;
};

template <typename Key, typename Value, size_t NTRY>
template <typename V> requires (!std::is_same_v<void, V>)
Key RingQueue<Key, Value, NTRY>::pop(V& value) {
    std::unique_lock<SpinLock> l(read_position_.lock);
    slot_t& slot = queue_[read_position_.i & & capacity_mask_];

    while(true){
        Key result = slot.key.load(std::memory_order_acquire);
        
        if(result == Key{}) return Key {};                      // Queue is empty

        if(!slot.busy.load(std::memory_order_acquire) &&
            slot.key.load(std::memory_order_acquire)){

            slot.busy_flag.store(1, std::memory_order_release); // Lock slot in busy state
            ++read_position_.i;                                 // Move position to next for other readers/consumers
            l.unlock();

            value = std::move(slot.value);
            slot.value.~Value();

            slot.key.store(Key{}, std::memory_order_release);   // Store empty value
            slot.busy.store(0, std::memory_order_release);      // Release by flag
            return result;
        }

        if(slot.busy.load(std::memory_order_acquire)) std::this_thread::yield();
    }
};

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