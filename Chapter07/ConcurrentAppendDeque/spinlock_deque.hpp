#pragma once

#include <cstddef>
#include <deque>
#include <mutex>

#include "../SpinLock/spinlock.hpp"

template <typename T>
class SpinlockDeque {
public:
    void push_back(const T& value) {
        std::lock_guard<SpinLock> lock(spinlock_);
        deque_.push_back(value);
    }

    void resize(std::size_t new_size) {
        std::lock_guard<SpinLock> lock(spinlock_);
        if (new_size > deque_.size()) {
            deque_.resize(new_size);
        }
    }

    std::size_t size() const {
        std::lock_guard<SpinLock> lock(spinlock_);
        return deque_.size();
    }

    T& operator[](std::size_t index) {
        std::lock_guard<SpinLock> lock(spinlock_);
        return deque_[index];
    }

    const T& operator[](std::size_t index) const {
        std::lock_guard<SpinLock> lock(spinlock_);
        return deque_[index];
    }

private:
    mutable SpinLock spinlock_;
    std::deque<T> deque_;
};
