#pragma once
#include "SPSCQueue.h"

namespace spscqueue {

    template <typename T>
    SPSCQueue<T>::SPSCQueue(size_t capacity) : capacity_(capacity), head_(0), tail_(0) {
        if (capacity < 2 || (capacity & (capacity - 1)) != 0)
            throw std::invalid_argument("Capacity must be >= 2 and a power of 2");
        buffer_ = static_cast<T*>(operator new[](capacity_ * sizeof(T)));
    }

    template <typename T>
    SPSCQueue<T>::~SPSCQueue() {
        size_t t = tail_.load(std::memory_order_relaxed);
        size_t h = head_.load(std::memory_order_relaxed);
        while (t != h) {
            buffer_[t].~T();
            t = (t + 1) & (capacity_ - 1);
        }
        operator delete[](buffer_);
    }

    template <typename T>
    bool SPSCQueue<T>::push(const T& item) {
        size_t h = head_.load(std::memory_order_relaxed);
        size_t next = (h + 1) & (capacity_ - 1);
        if (next == tail_.load(std::memory_order_acquire)) return false; // full
        new (&buffer_[h]) T(item); // placement new
        head_.store(next, std::memory_order_release);
        return true;
    }

    template <typename T>
    bool SPSCQueue<T>::pop(T& item) {
        size_t t = tail_.load(std::memory_order_relaxed);
        if (t == head_.load(std::memory_order_acquire)) return false; // empty
        item = std::move(buffer_[t]);
        buffer_[t].~T();
        tail_.store((t + 1) & (capacity_ - 1), std::memory_order_release);
        return true;
    }

    template <typename T>
    bool SPSCQueue<T>::full() const {
        size_t h = head_.load(std::memory_order_acquire);
        size_t next = (h + 1) & (capacity_ - 1);
        return next == tail_.load(std::memory_order_acquire);
    }

    template <typename T>
    bool SPSCQueue<T>::empty() const {
        return head_.load(std::memory_order_acquire) == tail_.load(std::memory_order_acquire);
    }

    template <typename T>
    size_t SPSCQueue<T>::size() const {
        size_t h = head_.load(std::memory_order_acquire);
        size_t t = tail_.load(std::memory_order_acquire);
        return (h - t) & (capacity_ - 1);
    }

    template <typename T>
    size_t SPSCQueue<T>::capacity() const {
        return capacity_;
    }

} // namespace spscqueue
