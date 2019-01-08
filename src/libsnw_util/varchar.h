#pragma once

#include <ostream>
#include <stdexcept>
#include <cassert>

namespace snw {

template<int capacity_>
class varchar {
    static_assert(0 < capacity_, "capacity must be greater than zero");
public:
    varchar() : data_{} {
        clear();
    }

    varchar(const char* str) : data_{} {
        assign(str);
    }

    varchar(const char* str, int len) : data_{} {
        assign(str, len);
    }

    varchar& operator=(const char* str) {
        assign(str);
    }

    bool empty() const {
        return data_[0] == '\0';
    }

    int size() const {
        int i = 0;
        for (; i < capacity_; ++i) {
            if (data_[i] == '\0') {
                break;
            }
        }

        return i;
    }

    int capacity() const {
        return capacity_;
    }

    const char* data() const {
        return data_;
    }

    const char* begin() const {
        return data_;
    }

    const char* end() const {
        return data_ + size();
    }

    void clear() {
        for (int i = 0; i < capacity_; ++i) {
            data_[i] = '\0';
        }
    }

    void assign(const char* str) {
        for (int i = 0; i < capacity_; ++i) {
            data_[i] = *str;
            if (*str != '\0') {
                str++;
            }
            else {
                // keep assigning '\0' from str
            }
        }

        if (*str != '\0') {
            throw std::runtime_error("varchar buffer overflow");
        }
    }

    void assign(const char* str, int len) {
        if (len > capacity_) {
            throw std::runtime_error("varchar buffer overflow");
        }

        int i = 0;
        for (; i < len; ++i) {
            data_[i] = str[i];
        }

        for (; i < capacity_; ++i) {
            data_[i] = '\0';
        }
    }

    char operator[](int index) const {
        assert(index < capacity_);
        return data_[index];
    }

public:
    friend int compare(const varchar& lhs, const varchar& rhs) {
        int rc = 0;
        for (int i = 0; i < capacity_; ++i) {
            rc = static_cast<int>(lhs[i]) - static_cast<int>(rhs[i]);
            if (rc) {
                break; // found a character that wasn't equal
            }
        }

        return rc;
    }

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

    friend std::ostream& operator<< (std::ostream& out, const varchar& val) {
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
