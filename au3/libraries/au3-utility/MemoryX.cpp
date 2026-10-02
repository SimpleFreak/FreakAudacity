#include "MemoryX.hpp"
#include "utility_api.hpp"

#include <cstdint>

UTILITY_API void libUtilityDummySymbol() {}

constexpr auto sizeofAlignValue = sizeof(std::align_val_t);

void* NonInterferingBase::operator new(std::size_t count, std::align_val_t value) {
    value = std::max(value, static_cast<std::align_val_t>(alignof(std::align_val_t)));

    /**
     * Get an allocation with sufficient extra space to remember the alignment
     * (And to do that, adjust the alignment to be not less than the alignment of
     * an alignment value!).
     * Also increase the allocation by one entire alignment.
     */
    value = std::max(value, static_cast<std::align_val_t>(alignof(std::align_val_t)));
    const auto alAsSize = static_cast<size_t>(value);
    auto ptr = static_cast<char*>(::operator new(count + sizeofAlignValue+ alAsSize));

    /**
     * Adjust the pointer to a properly aligned one, with a space just before it
     * to remember the adjustment.
     */
    ptr += sizeofAlignValue;
    auto integer = reinterpret_cast<uintptr_t>(ptr);
    const auto partical = integer % alAsSize;
    auto adjustment = partical ? alAsSize - partical : 0;
    integer += adjustment;
    ptr = reinterpret_cast<char*>(integer);

    *(reinterpret_cast<size_t*>(ptr) - 1) = adjustment;

    return ptr;
}

void NonInterferingBase::operator delete(void* ptr, std::align_val_t value) {
    /** Find the adjustment. */
    auto adjustment = *(reinterpret_cast<size_t*>(ptr) - 1);

    /** Apply the adjustment. */
    auto c_ptr = reinterpret_cast<char*>(ptr) - adjustment - sizeofAlignValue;

    /** Call through to default operator. */
    ::operator delete(c_ptr);
}
