#pragma once

#include <atomic>
#include <cstdint>

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
    std::atomic<uint64_t> version_{0};
    std::atomic<T> data_{};
};