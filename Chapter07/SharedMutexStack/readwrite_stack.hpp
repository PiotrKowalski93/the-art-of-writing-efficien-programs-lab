#pragma once

#include <mutex>
#include <optional>
#include <shared_mutex>
#include <stack>
#include <utility>

template <typename T>
class readwrite_stack {
public:
    void push(const T& obj) {
        std::unique_lock lock(mutex_);
        stack_.push(obj);
    }

    std::optional<T> pop() {
        std::unique_lock lock(mutex_);
        if (stack_.empty()) {
            return std::nullopt;
        }

        std::optional<T> result(std::move(stack_.top()));
        stack_.pop();
        return result;
    }

    std::optional<T> top() const {
        std::shared_lock lock(mutex_);
        if (stack_.empty()) {
            return std::nullopt;
        }
        return stack_.top();
    }

private:
    std::stack<T> stack_;
    mutable std::shared_mutex mutex_;
};
