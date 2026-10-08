#ifndef __AUDACITY_LOCK_FREE_QUEUE_HPP__
#define __AUDACITY_LOCK_FREE_QUEUE_HPP__

#include "MemoryX.hpp"

#include <atomic>
#include <cassert>
#include <cstdint>

/** Single-producer, single-consumer thread-safe queue of update messages. */
template<typename T>
class LockFreeQueue : public SharedNonInterfering<LockFreeQueue<T>> {
public:
    explicit LockFreeQueue(size_t maxLength);
    ~LockFreeQueue();

    bool Put(const T& message);
    bool Get(T& message);

    void Clear();

private:
    /**
     * Align the two atomics to avoid false sharing
     * m_start is written only by the reader, m_end by the writer.
     */
    NonInterfering<std::atomic<size_t>> m_start{0};
    NonInterfering<std::atomic<size_t>> m_end{0};

    const size_t m_bufferSize{};
    ArrayOf<T> m_buffer{m_bufferSize};
};

template<typename T>
LockFreeQueue<T>::LockFreeQueue(size_t maxLength) : m_bufferSize{maxLength} {
    Clear();
}

template<typename T> LockFreeQueue<T>::~LockFreeQueue() {}

template<typename T> void LockFreeQueue<T>::Clear() {
    m_start.store(0);
    m_end.store(0);
}

/**
 * Add a message to the end of the queue. Return false if the queue was full.
 */
template<typename T> bool LockFreeQueue<T>::Put(const T& message) {
    auto start = m_start.load(std::memory_order_acquire);
    auto end = m_end.load(std::memory_order_relaxed);

    /** m_start can be greater than m_end because it is all mod m_bufferSize. */
    assert((end + m_bufferSize - start) >= 0);
    int32_t length = (end + m_bufferSize - start) % m_bufferSize;

    /**
     * Never completely fill the queue, because then the state is ambiguous (m_start == m_end).
     */
    if (length + 1 >= (int32_t)(m_bufferSize)) {
        return false;
    }

    m_buffer[end] = message;
    m_end.store((end + 1) % m_bufferSize, std::memory_order_release);

    return true;
}

/**
 * Get the next message from the start of the queue.
 * Return false if the queue was empty.
 */
template<typename T> bool LockFreeQueue<T>::Get(T& message) {
    auto start = m_start.load(std::memory_order_relaxed);
    auto end = m_end.load(std::memory_order_acquire);
    int32_t length = (end + m_bufferSize - start) % m_bufferSize;

    if (length == 0) {
        return false;
    }

    msg = m_buffer[start];
    m_start.store((start + 1) % m_bufferSize, std::memory_order_release);

    return true;
}

#endif
