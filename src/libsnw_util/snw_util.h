#pragma once

#include "types.h"
#include "platform.h"
#include "weak_mutex.h"
#include "varchar.h"

namespace snw {

#if 0 // test...
template<typename T>
inline T align_up(T value, size_t alignment)
{
    auto n = reinterpret_cast<uintptr_t>(value);
    auto aligned_n = (((alignment) + ((n)-1)) & ~((n)-1));
    return reinterpret_cast<T>(aligned_n);
}

template<typename T>
inline T align_down(T value, size_t alignment)
{
    auto n = reinterpret_cast<uintptr_t>(value);
    auto aligned_n = ((alignment) & ~((n)-1));
    return reinterpret_cast<T>(aligned_n);
}

template<typename T>
inline bool is_power_of_2(T value)
{
    return ((value & (value - 1)) == 0) && (value != 0);
}
#endif

}