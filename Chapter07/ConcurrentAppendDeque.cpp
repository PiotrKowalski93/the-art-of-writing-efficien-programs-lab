#include <stddef.h>
#include <atomic>
#include <vector>
#include <time.h>

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

    static_assert(BlockSize > 0 && (BlockSize & (BlockSize - 1) == 0), "BlockSize must by power of 2");

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
    }
    
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
    T** new_dir = new T**[new_capacity];
    // We can use relaxed because we are locked
    T** old_dir = directory_.load(std::memory_order_relaxed);
    
    for(site_t i = 0; i < capacity_; ++i){
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