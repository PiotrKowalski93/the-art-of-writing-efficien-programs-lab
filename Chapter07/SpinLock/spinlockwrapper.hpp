#pragma once

#include <mutex>

#include "spinlock.hpp"

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