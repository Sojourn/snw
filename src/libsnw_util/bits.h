#pragma once

#include <cstdint>
#include <cassert>

namespace snw {

template<typename T>
inline void set_bit(T& value, int index) {
    value |= (static_cast<T>(1) << index);
}

template<typename T>
inline void clear_bit(T& value, int index) {
    value &= ~(static_cast<T>(1) << index);
}

template<typename T>
inline void toggle_bit(T& value, int index) {
    value ^= (static_cast<T>(1) << index);
}

template<typename T>
inline bool test_bit(T value, int index) {
    return value & (static_cast<T>(1) << index);
}

inline int count_trailing_zeros(uint32_t value) {
    return __builtin_ctz(value);
}

inline int count_trailing_zeros(uint64_t value) {
    return __builtin_ctzll(value);
}

inline int count_set_bits(uint32_t value) {
    return __builtin_popcount(value);
}

inline int count_set_bits(uint64_t value) {
    return __builtin_popcountll(value);
}

inline int extract_set_bits(uint32_t value, int (&result)[32]) {
    int set_bit_cnt = count_set_bits(value);
    for (uint32_t i = 0; i < set_bit_cnt; ++i) {
        int bit_index = count_trailing_zeros(value);
        value &= ~(static_cast<uint32_t>(1) << bit_index);
        result[i] = bit_index;
    }

    return set_bit_cnt;
}

inline int extract_set_bits(uint64_t value, int (&result)[64]) {
    int set_bit_cnt = count_set_bits(value);
    for (uint64_t i = 0; i < set_bit_cnt; ++i) {
        int bit_index = count_trailing_zeros(value);
        value &= ~(static_cast<uint64_t>(1) << bit_index);
        result[i] = bit_index;
    }

    return set_bit_cnt;
}

template<typename F>
inline void for_each_set_bit(uint32_t value, F&& f) {
    int result[32];
    int set_bit_cnt = extract_set_bits(value, result);
    for (int i = 0; i < set_bit_cnt; ++i) {
        f(result[i]);
    }
}

template<typename F>
inline void for_each_set_bit(uint64_t value, F&& f) {
    int result[64];
    int set_bit_cnt = extract_set_bits(value, result);
    for (int i = 0; i < set_bit_cnt; ++i) {
        f(result[i]);
    }
}

template<typename F>
inline void for_each_set_bit_volatile(const uint64_t& value, F&& f) {
    uint64_t mask = ~static_cast<uint64_t>(0);
    while (uint64_t masked_value = (value & mask)) {
        int index = count_trailing_zeros(masked_value);
        f(index);
        mask = ~((static_cast<uint64_t>(1) << (index + 1)) - 1);
    }
}

}
