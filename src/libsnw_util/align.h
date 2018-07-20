#pragma once

#include "types.h"

namespace snw {

template<typename T>
inline T align_up(T value, size_t alignment)
{
    auto n = static_cast<size_t>(value);
    auto aligned_n = (alignment + (n-1)) & ~(n-1);
    return static_cast<T>(aligned_n);
}

template<typename T>
inline T align_down(T value, size_t alignment)
{
    auto n = static_cast<size_t>(value);
    auto aligned_n = (alignment & ~(n-1));
    return static_cast<T>(aligned_n);
}

}