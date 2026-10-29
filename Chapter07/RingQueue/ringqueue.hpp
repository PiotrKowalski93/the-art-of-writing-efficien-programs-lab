#include <atomic>
#include <bit>
#include <cstddef>
#include <memory>
#include <new>
#include <thread>
#include <mutex>
#include <type_traits>
#include <utility>

#include "../SpinLock/spinlock.hpp"

// ------------------------------------------------------------------
// Implementation
// ------------------------------------------------------------------
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

        Key key = slot.key.load(std::memory_order_relaxed);
        return key == Key{};
    }

private:
    // General case with Key and Value
    template <typename K, typename V>
    struct alignas(256) queue_slot_t{
        std::atomic<K> key;
        std::atomic<int> busy_flag{0}; // Guard for value construction
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
    slot_t& slot = queue_[write_position_.i & capacity_mask_];

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
    slot_t& slot = queue_[read_position_.i & capacity_mask_];

    while(true){
        Key result = slot.key.load(std::memory_order_acquire);
        
        if(result == Key{}) return Key {};                      // Queue is empty

        if(!slot.busy_flag.load(std::memory_order_acquire) &&
            slot.key.load(std::memory_order_acquire) != Key{}){

            slot.busy_flag.store(1, std::memory_order_release); // Lock slot in busy state
            ++read_position_.i;                                 // Move position to next for other readers/consumers
            l.unlock();

            value = std::move(slot.value);
            slot.value.~Value();

            slot.key.store(Key{}, std::memory_order_release);   // Store empty value
            slot.busy.store(0, std::memory_order_release);      // Release by flag
            return result;
        }

        if(slot.busy_flag.load(std::memory_order_acquire)) std::this_thread::yield();
    }
};