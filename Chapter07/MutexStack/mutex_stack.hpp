#pragma once

#include <mutex>
#include <optional>
#include <stack>
#include <utility>

template <typename T>
class mutex_stack {
public:
    void push(const T& obj) {
        std::lock_guard lock(mutex_);
        stack_.push(obj);
    }

    std::optional<T> pop() {
        std::lock_guard lock(mutex_);
        if (stack_.empty()) {
            return std::nullopt;
        }

        std::optional<T> result(std::move(stack_.top()));
        stack_.pop();
        return result;
    }

    std::optional<T> top() const {
        std::lock_guard lock(mutex_);
        if (stack_.empty()) {
            return std::nullopt;
        }
        return stack_.top();
    }

private:
    std::stack<T> stack_;
    mutable std::mutex mutex_;
};
