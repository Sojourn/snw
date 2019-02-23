#pragma once

#include <limits>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include "align.h"
#include "object_ref.h"

namespace snw {

class object_heap {
public:
    object_heap()
        : size_(0)
        , old_size_(0)
    {
    }

    object_ref new_nil() {
        object_ref ref;
        ref.type = object_type::nil;
        return ref;
    }

    object_ref new_integer(int64_t value) {
        object_ref ref;
        ref.type = object_type::integer;
        if ((std::numeric_limits<int16_t>::min() <= value) && (value <= std::numeric_limits<int16_t>::max())) {
            ref.value.integer = static_cast<int16_t>(value);
        }
        else {
            check_free_space(sizeof(integer_repr));

            ref.is_indirect = true;
            ref.value.address = size_;

            auto& repr = access<integer_repr>(ref);
            repr.value = value;

            size_ += sizeof(value);
        }

        return ref;
    }

    object_ref new_symbol(const char* name) {
        return new_symbol(symbol(name));
    }

    object_ref new_symbol(const char* first, const char* last) {
        return new_symbol(symbol(first, last - first));
    }

    object_ref new_symbol(const symbol& name) {
        check_free_space(sizeof(symbol_repr));

        object_ref ref;
        ref.type = object_type::symbol;
        ref.is_indirect = true;
        ref.value.address = size_;

        auto& repr = access<symbol_repr>(ref);
        repr.name = name;

        size_ += sizeof(symbol);
        return ref;
    }

    object_ref new_string(const char* str) {
        return new_string(str, str + strlen(str));
    }

    object_ref new_string(const char* first, const char* last) {
        object_ref ref;
        ref.type = object_type::string;
        if (first != last) {
            size_t str_len = last - first;
            size_t alloc_size = align_up(sizeof(string_repr) + str_len + 1, alignment_);
            check_free_space(alloc_size);

            ref.is_indirect = true;
            ref.value.address = static_cast<uint16_t>(size_);

            auto& repr = access<string_repr>(ref);
            repr.len = str_len;
            memcpy(repr.str, first, str_len);
            repr.str[str_len] = '\0';

            size_ += alloc_size;
        }

        return ref;
    }

    object_ref new_bytes(const uint8_t* first, const uint8_t* last) {
        object_ref ref;
        ref.type = object_type::bytes;
        if (first != last) {
            size_t buf_len = last - first;
            size_t alloc_size = align_up(sizeof(bytes_repr) + buf_len, alignment_);
            check_free_space(alloc_size);

            ref.is_indirect = true;
            ref.value.address = static_cast<uint16_t>(size_);

            auto& repr = access<bytes_repr>(ref);
            repr.len = buf_len;
            memcpy(repr.buf, first, buf_len);

            size_ += alloc_size;
        }

        return ref;
    }

    object_ref new_cell(object_ref car, object_ref cdr) {
        object_ref ref;
        ref.type = object_type::cell;
        if ((car.type != object_type::nil) || (cdr.type != object_type::nil)) {
            check_free_space(sizeof(cell_repr));

            object_ref ref;
            ref.type = object_type::cell;
            ref.is_indirect = true;
            ref.value.address = static_cast<uint16_t>(size_);

            auto& repr = access<cell_repr>(ref);
            repr.car = car;
            repr.cdr = cdr;

            size_ += sizeof(cell_repr);
        }

        return ref;
    }

    object_ref new_list(const object_ref* first, const object_ref* last) {
        ssize_t len = last - first;
        size_t alloc_size = sizeof(cell_repr) * len;
        check_free_space(alloc_size);

        object_ref head = new_nil();
        for (ssize_t i = len - 1; i >= 0; --i) {
            object_ref cell_ref;
            cell_ref.type = object_type::cell;
            cell_ref.is_indirect = true;
            cell_ref.value.address = size_ + (i * sizeof(cell_repr));

            auto& repr = access<cell_repr>(cell_ref);
            repr.car = first[i];
            repr.cdr = (i < (len - 1)) ? head : new_nil();

            head = cell_ref;
        }

        size_ += alloc_size;
        return head;
    }

public:
    const integer_repr& deref_integer(object_ref ref) const {
        assert(ref.type == object_type::integer);
        assert(ref.is_indirect);
        return access<integer_repr>(ref);
    }

    const symbol_repr& deref_symbol(object_ref ref) const {
        assert(ref.type == object_type::symbol);
        assert(ref.is_indirect);
        return access<symbol_repr>(ref);
    }

    const string_repr& deref_string(object_ref ref) const {
        assert(ref.type == object_type::string);
        assert(ref.is_indirect);
        return access<string_repr>(ref);
    }

    const bytes_repr& deref_bytes(object_ref ref) const {
        assert(ref.type == object_type::bytes);
        assert(ref.is_indirect);
        return access<bytes_repr>(ref);
    }

    const cell_repr& deref_cell(object_ref ref) const {
        assert(ref.type == object_type::cell);
        assert(ref.is_indirect);
        return access<cell_repr>(ref);
    }

public:
    void begin() {
        assert(old_size_ == 0);
        old_size_ = size_;
    }

    void commit() {
        old_size_ = 0;
    }

    void rollback() {
        size_ = old_size_;
        old_size_ = 0;
    }

private:
    void check_free_space(size_t object_size) const {
        if (capacity_ < (size_ + object_size)) {
            // TODO: specialized exception type for this
            throw std::runtime_error("object heap out of memory");
        }
    }

    template<typename T>
    T& access(object_ref ref) {
        assert(ref.is_indirect);
        return *reinterpret_cast<T*>(&data_[ref.value.address]);
    }

    template<typename T>
    const T& access(object_ref ref) const {
        assert(ref.is_indirect);
        return *reinterpret_cast<const T*>(&data_[ref.value.address]);
    }

private:
    static constexpr size_t capacity_ = 1 << 16;
    static constexpr size_t alignment_ = 8;

    size_t  size_;
    size_t  old_size_;
    uint8_t data_[capacity_];
};

}
