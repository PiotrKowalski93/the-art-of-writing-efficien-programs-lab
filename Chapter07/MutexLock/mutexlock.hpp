#pragma once

#include <mutex>

template <typename T>
class MutexLock {
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
    T data_{};
};