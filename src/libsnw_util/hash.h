#pragma once

#include <cstdint>
#include <cstring>
#include "varchar.h"

namespace snw {

// stolen from https://nullprogram.com/blog/2018/07/31/
inline uint32_t hash32(uint32_t x) {
    static constexpr uint32_t a = 0xed5ad4bb;
    static constexpr uint32_t b = 0xac4c1b51;
    static constexpr uint32_t c = 0x31848bab;

    x ^= x >> 17;
    x *= a;
    x ^= x >> 11;
    x *= b;
    x ^= x >> 15;
    x *= c;
    x ^= x >> 14;

    return x;
}

template<size_t capacity>
inline uint32_t hash32(const varchar<capacity>& value) {
    uint32_t hash = 0;
    for (size_t i = 0; i < capacity; i += sizeof(hash)) {
        uint32_t x;
        memcpy(&x, &value[i], sizeof(x));
        hash ^= hash32(x);
    }

    return hash;
}

}
