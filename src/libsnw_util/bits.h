#pragma once

#include <cassert>
#include "platform.h"

namespace snw {

inline uint32_t count_trailing_zeros(uint32_t value) {
    assert(value);

#ifdef SNW_OS_LINUX
    static_assert(sizeof(value) == sizeof(unsigned int),
        "Using the wrong builtin for this type");

    return static_cast<uint32_t>(__builtin_ctz(value));

#else
    for (uint32_t i = 0; i < (sizeof(uint32_t) * 8); ++i) {
        if (value & (static_cast<uint32_t>(1) << i)) {
            return i;
        }
    }

    return 0;
#endif
}

inline uint64_t count_trailing_zeros(uint64_t value) {
    assert(value);

#ifdef SNW_OS_LINUX
    static_assert(sizeof(value) == sizeof(unsigned long long),
        "Using the wrong builtin for this type");

    return static_cast<uint64_t>(__builtin_ctzll(value));

#else
    for (uint32_t i = 0; i < (sizeof(uint64_t) * 8); ++i) {
        if (value & (static_cast<uint64_t>(1) << i)) {
            return i;
        }
    }

    return 0;
#endif
}

}