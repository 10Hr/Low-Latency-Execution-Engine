#pragma once
#include <atomic>
#include <cstddef>
#include <stdexcept>
#include <new> // for operator new[]

namespace spscqueue {

template <typename T>
class SPSCQueue {
public:
    explicit SPSCQueue(size_t capacity);
    ~SPSCQueue();

    SPSCQueue(const SPSCQueue&) = delete;
    SPSCQueue& operator=(const SPSCQueue&) = delete;
    SPSCQueue(SPSCQueue&&) = delete;
    SPSCQueue& operator=(SPSCQueue&&) = delete;

    bool push(const T& item);
    bool pop(T& item);

    [[nodiscard]] bool full() const;
    [[nodiscard]] bool empty() const;
    [[nodiscard]] size_t size() const;
    [[nodiscard]] size_t capacity() const;

private:
    const size_t capacity_;
    T* buffer_;
    std::atomic<size_t> head_;
    std::atomic<size_t> tail_;
};

#include "SPSCQueue.tpp" // include template implementation

} // namespace spscqueue
