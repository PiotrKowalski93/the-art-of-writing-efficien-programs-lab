#include <cstdint>
#include <atomic>
#include <barrier>
#include <thread>
#include <mutex>
#include <queue>
#include <optional>
#include <benchmark/benchmark.h>

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

// TODO: Possible optimalization is adding alignas to slots type
// it might help with false sharing between producer and consumer on adjacent slots
template <typename Key, typename Value = void, size_t NTRY = 8>  // Number of attempts to acquire a slot before giving up
class RingQueue {
public:
    explicit RingQueue(size_t capacity);
    RingQueue(size_t capacity, void* memory);
    ~RingQueue();

    template <typename V> requires std::is_constructible_v<Value,V>
    bool push(Key key, V&& value);
    bool push(Key key) requires std::is_same_v<Value, void>;
    template <typename V = Value> requires (!std::is_same_v<void, V>)
    Key pop(V& value);
    Key pop() requires std::is_same_v<Value, void>;
    
    size_t element_size() const { 
        return sizeof(slot_t);  
    };

    size_t capacity() const {
        return capacity_;
    };

    bool empty() const;


private:
    // General case with Key and Value
    template <typename K, typename V>
    struct queue_slot_t{
        std::atomic<K> key;
        std::atomic<int> busy_flag; // Guard for value contruction
        V value;
    };

    // For fast path we store only Key
    template <typename K>
    struct queue_slot_t<K, void> {
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
}

template <typename Key, typename Value, size_t NTRY>
Key RingQueue<Key, Value, NTRY>::pop() requires std::is_same_v<Value, void> {
    std::unique_lock<SpinLock> l(read_position_.lock);
    slot_t& slot = queue_[read_position_.i & capacity_mask_];

    Key ret = slot.key.load(std::memory_order_acquire);
    if (ret == Key{}) return Key{};
    slot.key.store(Key{}, std::memory_order_release);
    
    ++read_position_.i;
    return ret;
}

BENCHMARK_MAIN();