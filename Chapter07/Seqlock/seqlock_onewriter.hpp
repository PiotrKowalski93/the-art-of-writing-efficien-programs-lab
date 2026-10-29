#pragma once

#include <atomic>
#include <cstdint>

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
    std::atomic<uint64_t> version_{0};
    std::atomic<T> data_{};
};