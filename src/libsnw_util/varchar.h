#pragma once

#include <ostream>
#include <stdexcept>
#include <cstddef>
#include <cassert>
#include "align.h"
#include "hash.h"

namespace snw {

template<size_t capacity_>
class varchar;

template<size_t capacity_>
int compare(const varchar<capacity_>& lhs, const varchar<capacity_>& rhs);

template<size_t capacity_>
class varchar {
    static_assert(0 < capacity_, "capacity must be greater than zero");
    static_assert(is_aligned(capacity_, 16), "capacity must be a multiple of 16");
public:
    varchar();
    varchar(const char* str);
    varchar(const char* str, size_t len);
    varchar(const varchar&) = default;

    varchar& operator=(const varchar&) = default;
    varchar& operator=(const char* str);

    bool empty() const;
    size_t size() const;
    size_t capacity() const;

    const char* data() const;
    const char* begin() const;
    const char* end() const;

    const char& at(size_t index) const;
    const char& operator[](size_t index) const;

    void clear();
    void assign(const char* str);
    void assign(const char* str, size_t len);

public:
    friend bool operator==(const varchar& lhs, const varchar& rhs) {
        return compare(lhs, rhs) == 0;
    }

    friend bool operator!=(const varchar& lhs, const varchar& rhs) {
        return compare(lhs, rhs) != 0;
    }

    friend bool operator<(const varchar& lhs, const varchar& rhs) {
        return compare(lhs, rhs) < 0;
    }

    friend bool operator<=(const varchar& lhs, const varchar& rhs) {
        return compare(lhs, rhs) <= 0;
    }

    friend bool operator>(const varchar& lhs, const varchar& rhs) {
        return compare(lhs, rhs) > 0;
    }

    friend bool operator>=(const varchar& lhs, const varchar& rhs) {
        return compare(lhs, rhs) >= 0;
    }

    friend std::ostream& operator<<(std::ostream& out, const varchar& val) {
        out.write(val.data(), val.size());
        return out;
    }

private:
    char data_[align_up(capacity_, 16)];
};

}

namespace std {
template<size_t capacity> struct hash<snw::varchar<capacity>> {
    using argument_type = snw::varchar<capacity>;
    using result_type = uint32_t;

    result_type operator()(const argument_type& value) const noexcept {
        result_type hash = 0;
        uint32_t chunk;
        for (size_t i = 0; i < capacity; i += sizeof(chunk)) {
            memcpy(&chunk, &value[i], sizeof(chunk));
            hash ^= snw::hash32(chunk);
        }

        return hash;
    }
};

}

#include "varchar.hpp"
