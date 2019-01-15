#pragma once

#include "types.h"

namespace snw {

constexpr size_t align_up(size_t value, size_t alignment) {
    return (value + (alignment - 1)) & ~(alignment - 1);
}

template<typename T>
constexpr T* align_up(T* pointer, size_t alignment)
{
    return reinterpret_cast<T*>(
        (reinterpret_cast<uintptr_t>(pointer) + (static_cast<uintptr_t>(alignment) - 1)) & ~(static_cast<uintptr_t>(alignment) - 1)
    );
}

constexpr size_t align_down(size_t value, size_t alignment) {
    return value & ~(alignment - 1);
}

template<typename T>
constexpr T* align_down(T* pointer, size_t alignment)
{
    return reinterpret_cast<T>(
        reinterpret_cast<uintptr_t>(pointer) & ~(static_cast<uintptr_t>(alignment) - 1)
    );
}

constexpr bool is_aligned(size_t value, size_t alignment) {
    return (value & (alignment - 1)) == 0;
}

constexpr bool is_aligned(const void* pointer, size_t alignment) {
    return (reinterpret_cast<uintptr_t>(pointer) & (alignment - 1)) == 0;
}

constexpr bool is_power_of_2(size_t value)
{
    return ((value & (value - 1)) == 0) && (value != 0);
}

}
