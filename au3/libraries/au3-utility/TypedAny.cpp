#include "TypedAny.hpp"

#include <cstdint>

namespace audacity {
    void checkedTypedAny() {
        TypedAny<int32_t> typedAny{};
        typedAny.reset();
    }
}
