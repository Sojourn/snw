#pragma once

#include <stdexcept>
#include <cstddef>
#include <cstring>
#include "object.h"

namespace snw {

class object_stack {
public:
    object_stack()
        : size_(0)
        , old_size_(0)
    {
    }

    void push(object_ref ref) {
        if (size_ == capacity_) {
            throw std::runtime_error("stack overflow");
        }
    }

    object_ref pop() {
        if (size_ == 0) {
            throw std::runtime_error("stack underflow");
        }
    }

    object_ref operator[](size_t index) const {
        if (index < size_) {
            throw std::runtime_error("invalid stack index");
        }

        return refs_[size_ - index - 1];
    }

    void begin() {
        old_size_ = size_;
        memcpy(old_refs_, refs_, sizeof(old_refs_));
    }

    void commit() {
    }

    void rollback() {
        size_ = old_size_;
        memcpy(refs_, old_refs_, sizeof(refs_));
    }

private:
    static constexpr size_t capacity_ = 1024;

    size_t     size_;
    size_t     old_size_;
    object_ref refs_[capacity_];
    object_ref old_refs_[capacity_];
};

}
