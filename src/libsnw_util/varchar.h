#pragma once

#include <ostream>
#include <stdexcept>
#include <cassert>

namespace snw {

template<int capacity_>
class varchar;

template<int capacity_>
int compare(const varchar<capacity_>& lhs, const varchar<capacity_>& rhs);

template<int capacity_>
class varchar {
    static_assert(0 < capacity_, "capacity must be greater than zero");
public:
    varchar();
    varchar(const char* str);
    varchar(const char* str, int len);
    varchar(const varchar&) = default;

    varchar& operator=(const varchar&) = default;
    varchar& operator=(const char* str);

    bool empty() const;
    int size() const;
    int capacity() const;

    const char* data() const;
    const char* begin() const;
    const char* end() const;

    const char& at(int index) const;
    const char& operator[](int index) const;

    void clear();
    void assign(const char* str);
    void assign(const char* str, int len);

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
    char data_[capacity_];
};

using varchar8  = varchar<8>;
using varchar16 = varchar<16>;
using varchar32 = varchar<32>;
using varchar64 = varchar<64>;

}

#include "varchar.hpp"
