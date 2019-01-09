#pragma once

#include <ostream>
#include <stdexcept>
#include <cstddef>
#include <cassert>
#include "align.h"

namespace snw {

template<size_t capacity_>
class varchar;

template<size_t capacity_>
int compare(const varchar<capacity_>& lhs, const varchar<capacity_>& rhs);

template<size_t capacity_>
class varchar {
    static_assert(0 < capacity_, "capacity must be greater than zero");
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
    alignas(16) char data_[align_up(capacity_, 16)];
};

}

#include "varchar.hpp"
