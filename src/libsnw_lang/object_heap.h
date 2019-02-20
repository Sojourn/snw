#pragma once

#include <limits>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include "align.h"
#include "object.h"

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
            check_free_space(sizeof(value));

            ref.is_indirect = true;
            ref.value.address = size_;

            access<int64_t>(ref) = value;

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

    object_ref new_symbol(const symbol& value) {
        check_free_space(sizeof(value));

        object_ref ref;
        ref.type = object_type::symbol;
        ref.is_indirect = true;
        ref.value.address = size_;

        access<symbol>(ref) = value;

        size_ += sizeof(value);
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
            size_t alloc_size = align_up(sizeof(string_object) + str_len + 1, alignment_);
            check_free_space(alloc_size);

            ref.is_indirect = true;
            ref.value.address = static_cast<uint16_t>(size_);

            auto& obj = access<string_object>(ref);
            obj.len = str_len;
            memcpy(obj.str, first, str_len);
            obj.str[str_len] = '\0';

            size_ += alloc_size;
        }

        return ref;
    }

    object_ref new_bytes(const uint8_t* first, const uint8_t* last) {
        object_ref ref;
        ref.type = object_type::bytes;
        if (first != last) {
            size_t buf_len = last - first;
            size_t alloc_size = align_up(sizeof(bytes_object) + buf_len, alignment_);
            check_free_space(alloc_size);

            ref.is_indirect = true;
            ref.value.address = static_cast<uint16_t>(size_);

            auto& obj = access<bytes_object>(ref);
            obj.len = buf_len;
            memcpy(obj.buf, first, buf_len);

            size_ += alloc_size;
        }

        return ref;
    }

    object_ref new_cell(object_ref car, object_ref cdr) {
        object_ref ref;
        ref.type = object_type::cell;
        if ((car.type != object_type::nil) || (cdr.type != object_type::nil)) {
            check_free_space(sizeof(cell_object));

            object_ref ref;
            ref.type = object_type::cell;
            ref.is_indirect = true;
            ref.value.address = static_cast<uint16_t>(size_);

            cell_object cell;
            cell.car = car;
            cell.cdr = cdr;
            access<cell_object>(ref) = cell;

            size_ += sizeof(cell_object);
        }

        return ref;
    }

    object_ref new_list(const object_ref* first, const object_ref* last) {
        ssize_t len = last - first;
        size_t alloc_size = sizeof(cell_object) * len;
        check_free_space(alloc_size);

        object_ref head = new_nil();
        for (ssize_t i = len - 1; i >= 0; --i) {
            object_ref cell_ref;
            cell_ref.type = object_type::cell;
            cell_ref.is_indirect = true;
            cell_ref.value.address = size_ + (i * sizeof(cell_object));

            cell_object cell;
            cell.car = first[i];
            if (i < (len - 1)) {
                cell.cdr = head;
            }
            else {
                // last list item
                cell.cdr = new_nil();
            }
            access<cell_object>(cell_ref) = cell;

            head = cell_ref;
        }

        size_ += alloc_size;
        return head;
    }

public:
    int64_t deref_integer(object_ref ref) {
        assert(ref.type == object_type::integer);
        if (ref.is_indirect) {
            return access<int64_t>(ref);
        }
        else {
            return ref.value.integer;
        }
    }

    const symbol& deref_symbol(object_ref ref) {
        assert(ref.type == object_type::symbol);
        if (ref.is_indirect) {
            return access<symbol>(ref);
        }
        else {
            throw std::runtime_error("invalid object reference");
        }
    }

    const string_object& deref_string(object_ref ref) {
        static const string_object empty_string = { 0 };

        assert(ref.type == object_type::string);
        if (ref.is_indirect) {
            return access<string_object>(ref);
        }
        else {
            return empty_string;
        }
    }

    const bytes_object& deref_bytes(object_ref ref) {
        static const bytes_object empty_bytes = { 0 };

        assert(ref.type == object_type::bytes);
        if (ref.is_indirect) {
            return access<bytes_object>(ref);
        }
        else {
            return empty_bytes;
        }
    }

    const cell_object& deref_cell(object_ref ref) {
        static const cell_object empty_cell;

        assert(ref.type == object_type::cell);
        if (ref.is_indirect) {
            return access<cell_object>(ref);
        }
        else {
            return empty_cell;
        }
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
