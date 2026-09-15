#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>

template <typename T>
class ThreadSafeQueue {
public:
    void push(T value) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(std::move(value));
        }
        cv_.notify_one();
    }

    std::optional<T> waitPop(std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (!cv_.wait_for(lock, timeout, [this] { return !queue_.empty(); }))
            return std::nullopt;

        T value = std::move(queue_.front());
        queue_.pop();
        return value;
    }

    std::optional<T> tryPop() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty())
            return std::nullopt;

        T value = std::move(queue_.front());
        queue_.pop();
        return value;
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

private:
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::queue<T> queue_;
};
