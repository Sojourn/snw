#pragma once

#include "types.h"

namespace snw {

template<typename T>
constexpr T align_up(T value, size_t alignment)
{
    return static_cast<T>(
        (static_cast<uintptr_t>(alignment) + (static_cast<uintptr_t>(value) - 1)) & ~(static_cast<uintptr_t>(value) - 1)
    );
}

template<typename T>
constexpr T align_down(T value, size_t alignment)
{
    return static_cast<T>(
        (static_cast<uintptr_t>(alignment) & ~(static_cast<uintptr_t>(value) - 1))
    );
}

}
