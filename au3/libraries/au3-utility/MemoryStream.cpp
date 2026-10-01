#include "MemoryStream.hpp"

#include <algorithm>

void MemoryStream::Clear() {
    m_chunks = {};
    m_linearData = {};
    m_dataSize = {};
}

void MemoryStream::AppendByte(char data) {
    AppendData(&data, 1);
}

void MemoryStream::AppendData(const void *data, const size_t length) {
    if (m_chunks.empty()) {
        m_chunks.emplace_back();
    }

    StreamChunk dataView = {data, length};

    while (m_chunks.back().Append(dataView) > 0) {
        m_chunks.emplace_back();
    }

    m_dataSize += length;
}

const void *MemoryStream::GetData() const {
    if (!m_chunks.empty()) {
        const size_t desiredSize = GetSize();

        m_linearData.reserve(desiredSize);

        for (const Chunk &chunk : m_chunks) {
            auto begin = chunk.Data.begin();
            auto end = begin + chunk.BytesUsed;

            m_linearData.insert(m_linearData.end(), begin, end);
        }

        m_chunks = {};
    }

    return m_linearData.data();
}

size_t MemoryStream::GetSize() const noexcept {
    return m_dataSize;
}

size_t MemoryStream::Chunk::Append(StreamChunk &dataView) {
    const size_t dataSize = dataView.second;

    const size_t bytesToWrite = std::min(ChunkSize - BytesUsed, dataSize);
    const size_t bytesLeft = dataSize - bytesToWrite;

    const uint8_t *beginData = static_cast<const uint8_t *>(dataView.first);
    const uint8_t *endData = beginData + bytesToWrite;

    /**
     * Some extreme micro optimization for MSVC (at least)
     * std::copy will generate a call to memmove in any case
     * which will be slower than just writing the byte.
     */
    if (bytesToWrite == 1) {
        Data[BytesUsed] = *beginData;
    } else {
        /**
         * There is a chance for unaligned access, so we do no handle
         * types as int32_t separately. Unaligned access is slow on x86
         * and fails horribly on ARM.
         */
        std::copy(beginData, endData, Data.begin() + BytesUsed);
    }

    dataView.first = endData;
    dataView.second = bytesLeft;

    BytesUsed += bytesToWrite;

    return bytesLeft;
}

bool MemoryStream::IsEmpty() const noexcept {
    return m_dataSize == 0;
}

MemoryStream::Iterator MemoryStream::begin() const {
    return Iterator(this, true);
}

MemoryStream::Iterator MemoryStream::end() const {
    return Iterator(this, false);
}

MemoryStream::Iterator::Iterator(const MemoryStream *stream, bool isBegin)
    : m_memoryStream(stream),
    m_listIterator(isBegin ? m_memoryStream->m_chunks.cbegin() : m_memoryStream->m_chunks.cend()),
    m_showLinearPart(isBegin && m_memoryStream->m_linearData.size() > 0) {}

MemoryStream::Iterator &MemoryStream::Iterator::operator++() {
    if (m_showLinearPart) {
        m_showLinearPart = false;
    } else {
        ++m_listIterator;
    }

    return *this;
}

MemoryStream::Iterator MemoryStream::Iterator::operator++(int32_t) {
    Iterator result{*this};
    this->operator++();

    return result;
}

MemoryStream::StreamChunk MemoryStream::Iterator::operator*() const {
    if (m_showLinearPart) {
        return {m_memoryStream->m_linearData.data(), m_memoryStream->m_linearData.size()};
    }

    return {m_listIterator->Data.data(), m_listIterator->BytesUsed};
}

MemoryStream::StreamChunk MemoryStream::Iterator::operator->() const {
    return this->operator*();
}

bool MemoryStream::Iterator::operator!=(const Iterator &rhs) const noexcept {
    return !(*this == rhs);
}

bool MemoryStream::Iterator::operator==(const Iterator &rhs) const noexcept {
    return m_memoryStream == rhs.m_memoryStream && m_listIterator == rhs.m_listIterator
           && m_showLinearPart == rhs.m_showLinearPart;
}
