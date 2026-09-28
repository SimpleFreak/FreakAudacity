#ifndef __AUDACITY_MEMORY_STREAM_HPP__
#define __AUDACITY_MEMORY_STREAM_HPP__

#include "utility_api.hpp"

#include <array>
#include <cstdint>
#include <list>
#include <vector>

#include "IteratorX.hpp"

/*!
 * @brief A low overhead memory stream with O(1) append, low heap fragmentation and a linear memory view.
 *
 * wxMemoryBuffer always appends 1Kb to the end of the buffer, causing severe performance issues
 * and significant heap fragmentation. There is no possibility to control the increment value.
 *
 * std::vector doubles its memory size which can be problematic for large projects as well.
 * Not as bad as wxMemoryBuffer though.
 *
 */
class UTILITY_API MemoryStream final {
public:
    using StreamData = std::vector<uint8_t>;
    using StreamChunk = std::pair<const void*, size_t>;

private:
    static constexpr size_t ChunkSize = 1024 * 1024 - 2 * sizeof(void*) - sizeof(size_t);

    struct Chunk final {
        std::array<uint8_t, ChunkSize> Data;
        size_t BytesUsed{0};

        /** Returns data size left to append. */
        size_t Append(StreamChunk& dataView);
    };

    using ChunksList = std::list<Chunk>;

public:
    MemoryStream() = default;
    MemoryStream(MemoryStream&&) = default;

    void Clear();

    void AppendByte(char data);
    void AppendData(const void* data, const size_t length);

    /**
     * This function possibly has O(size) complexity as it may
     * require copying bytes to a linear chunk
     */
    const void* GetData() const;
    size_t GetSize() const noexcept;

    bool IsEmpty() const noexcept;

    struct UTILITY_API Iterator : ValueIterator<const StreamChunk, std::forward_iterator_tag> {
        Iterator(const Iterator&) = default;

        Iterator& operator++();

        Iterator operator++(int32_t);

        StreamChunk operator*() const;
        StreamChunk operator->() const;

        bool operator==(const Iterator& rhs) const noexcept;
        bool operator!=(const Iterator& rhs) const noexcept;

    private:
        Iterator(const MemoryStream* stream, bool isBegin);

        const MemoryStream* m_memoryStream{nullptr};
        ChunksList::const_iterator m_listIterator{};
        bool m_showLinearPart{false};

        friend class MemoryStream;
    };

    Iterator begin() const;
    Iterator end() const;

private:
    /** This structures are lazily updated by get data. */
    mutable ChunksList m_chunks;
    mutable StreamData m_linearData;

    size_t m_dataSize{0};
};

#endif
