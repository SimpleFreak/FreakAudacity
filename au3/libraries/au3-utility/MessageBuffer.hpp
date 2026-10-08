#ifndef __AUDACITY_MESSAGE_BUFFER_HPP__
#define __AUDACITY_MESSAGE_BUFFER_HPP__

#include "MemoryX.hpp"

#include <atomic>
#include <cstdint>

/**
 * Communicate data atomically from one writer thread to one reader.
 *
 * This is not a queue: it is not necessary for each write to be read.
 * Rather loss of a message is allowed:  writer may overwrite.
 * Data must be default-constructible and reassignable.
 */
template <typename Data>
class MessageBuffer {
public:
    void Initialize();

    /**
     * ove data out (if available), or else copy it out
     *
     * @tparam Result is constructible from Data&& and forwards of other arguments
     */
    template <typename Result = Data, typename ... ConstructorArgs>
    Result Read(ConstructorArgs && ... args);

    /** Reassign a slot by move or copy. */
    template <typename Arg = Data&&>
    void Write(Arg&& arg);

private:
    struct UpdateSlot {
        Data m_data{};
        std::atomic<bool> m_busy{false};
    };

    NonInterfering<UpdateSlot> m_slots[2]{};

    std::atomic<unsigned char> m_lastWrittenSlot{0};
};

template <typename Data>
void MessageBuffer<Data>::Initialize() {
    for (auto& slot : m_slots) {
        while (slot.m_busy.exchange(true, std::memory_order_acquire)) {}
    }

    m_slots[0].m_data = {};
    m_slots[1].m_data = {};
    m_lastWrittenSlot.store(0, std::memory_order_relaxed);

    for (auto& slot : m_slots) {
        slot.m_busy.exchange(false, std::memory_order_release);
    }
}

template <typename Data>
template <typename Result, typename ... ConstructorArgs>
Result MessageBuffer<Data>::Read(ConstructorArgs&& ... args) {
    /** Whichever slot was last written, prefer to read that. */
    auto idx = m_lastWrittenSlot.load(std::memory_order_relaxed);
    idx = 1 - idx;
    bool wasBusy = false;

    while (true) {
        /**
         * This loop is unlikely to execute twice, but it might because the
         * producer thread is writing a slot.
         */
        idx = 1 - idx;
        wasBusy = m_slots[idx].m_busy.exchange(true, std::memory_order_acquire);

        if (!wasBusy) {
            break;
        }
    }

    /** Copy the slot. */
    Result result(std::move(m_slots[idx].m_data), std::forward<ConstructorArgs>(args) ...);

    m_slots[idx].m_busy.store(false, std::memory_order_release);

    return result;
}

template<typename Data>
template<typename Arg>
void MessageBuffer<Data>::Write(Arg&& arg) {
    /** Whichever slot was last written, prefer to write the other. */
    auto idx = m_lastWrittenSlot.load(std::memory_order_relaxed);
    bool wasBusy = false;

    while (true) {
        /**
         * This loop is unlikely to execute twice, but it might because the
         * consumer thread is reading a slot.
         */
        idx = 1 - idx;
        wasBusy = m_slots[idx].m_busy.exchange(true, std::memory_order_acquire);

        if (!wasBusy) {
            break;
        }
    }

    m_slots[idx].m_data = std::forward<Arg>(arg);
    m_lastWrittenSlot.store(idx, std::memory_order_relaxed);

    m_slots[idx].m_busy.store(false, std::memory_order_release);
}

#endif
