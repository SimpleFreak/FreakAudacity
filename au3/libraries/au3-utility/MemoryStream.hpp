#ifndef __MEMORY_STREAM_HPP__
#define __MEMORY_STREAM_HPP__

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


public:

};

#endif
