#pragma once

#include <stdexcept>
#include <cstddef>
#include <cstring>
#include <cassert>
#include "align.h"
#include "object_handle.h"

namespace snw {

class object_heap {
public:
    object_heap()
        : size_(0)
        , data_(new uint8_t[capacity_])
    {
        ::memset(data_.get(), 0, capacity_);

        setup();
    }

    void reset() {
        // zero previously allocated heap memory
        ::memset(data_.get(), 0, size_);
        size_ = 0;

        setup();
    }

    // allocate heap memory
    object_handle allocate(object_type type, size_t alloc_size) {
        assert(alloc_size > 0); // no zero-sized allocations

        alloc_size = align_up(alloc_size, alignment_);
        if (capacity_ < (size_ + alloc_size)) {
            throw std::runtime_error("object_heap allocation failed");
        }

        uint16_t addr = static_cast<uint16_t>(size_);
        size_ += alloc_size;

        object_handle handle;
        handle.set_type(type);
        handle.set_addr(addr);
        return handle;
    }

    // access heap memory
    template<typename T>
    T& access(object_handle handle) {
        assert(handle.addr() < size_);
        return *reinterpret_cast<T*>(&data_[handle.addr()]);
    }

    // access heap memory
    template<typename T>
    const T& access(object_handle handle) const {
        assert(handle.addr() < size_);
        return *reinterpret_cast<const T*>(&data_[handle.addr()]);
    }

private:
    // initialize an empty heap
    void setup() {
        assert(size_ == 0);

        allocate(object_type::nil, 1);
    }

private:
    static constexpr size_t    capacity_ = (1 << 16);
    static constexpr size_t    alignment_ = 8;

    size_t                     size_;
    std::unique_ptr<uint8_t[]> data_;
};

}