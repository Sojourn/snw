#pragma once

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cassert>
#include "object_handle.h"
#include "object_heap.h"

namespace snw {

template<object_type type>
class object;

template<>
class object<object_type::nil> : public object_handle {
    template<object_type type>
    friend object<type> object_cast(object_handle);

    template<object_type type>
    friend object<type> unsafe_object_cast(object_handle);

    template<object_type type, typename... Args>
    friend object<type> make_object(object_heap&, Args&&...);

    static constexpr uint16_t addr_ = 0;

private:
    static object create(object_heap& heap) {
        return object(object_handle(heap, object_type::nil, addr_));
    }

    object(object_handle handle)
        : object_handle(std::move(handle))
    {
    }
};

template<>
class object<object_type::boolean> : public object_handle {
    template<object_type type>
    friend object<type> object_cast(object_handle);

    template<object_type type>
    friend object<type> unsafe_object_cast(object_handle);

    template<object_type type, typename... Args>
    friend object<type> make_object(object_heap&, Args&&...);

    static constexpr uint16_t false_addr_ = 8;
    static constexpr uint16_t true_addr_  = 16;

public:
    bool value() const {
        return addr() == true_addr_;
    }

private:
    static object create(object_heap& heap, bool value) {
        return object(object_handle(heap, object_type::boolean, value ? true_addr_ : false_addr_));
    }

    object(object_handle handle)
        : object_handle(std::move(handle))
    {
    }
};

template<>
class object<object_type::integer> : public object_handle {
    template<object_type type>
    friend object<type> object_cast(object_handle);

    template<object_type type>
    friend object<type> unsafe_object_cast(object_handle);

    template<object_type type, typename... Args>
    friend object<type> make_object(object_heap&, Args&&...);

public:
    int64_t value() const {
        return heap().access<int64_t>(*this);
    }

    void set_value(int64_t value) {
        heap().access<int64_t>(*this) = value;
    }

private:
    static object create(object_heap& heap, int64_t value) {
        object result(heap.allocate(object_type::integer, sizeof(value)));
        result.set_value(value);
        return result;
    }

    object(object_handle handle)
        : object_handle(std::move(handle))
    {
    }
};

template<>
class object<object_type::symbol> : public object_handle {
public:
};

template<>
class object<object_type::string> : public object_handle {
public:
};

template<>
class object<object_type::cell> : public object_handle {
    template<object_type type>
    friend object<type> object_cast(object_handle);

    template<object_type type>
    friend object<type> unsafe_object_cast(object_handle);

    template<object_type type, typename... Args>
    friend object<type> make_object(object_heap&, Args&&...);

    struct cell {
        raw_object_handle car;
        raw_object_handle cdl;
    };

public:

private:
};

template<object_type type, typename... Args>
object<type> make_object(object_heap& heap, Args&&... args) {
    return object<type>::create(heap, std::forward<Args>(args)...);
}

template<object_type type>
object<type> unsafe_object_cast(object_handle handle) {
    assert(handle.type() == type);
    return object<type>(std::move(handle));
}

template<object_type type>
object<type> object_cast(object_handle handle) {
    if (handle.type() != type) {
        throw std::runtime_error("Invalid object cast");
    }

    return object<type>(std::move(handle));
}

#if 0
template<object_type type>
class object;

template<>
class object<object_type::nil> {
public:
    object(object_heap& heap, object_handle handle)
        : heap_(&heap)
        , handle_(handle)
    {
        assert(handle.type() == object_type::nil);
    }

    object(object_heap& heap)
        : heap_(&heap)
    {
        // collusion between the heap and nil objects
        handle_.set_type(object_type::nil);
        handle_.set_addr(0);
    }

private:
    object_heap*  heap_;
    object_handle handle_;
};

template<>
class object<object_type::integer> : public object_base {
public:
    object(object_heap& heap, object_handle handle)
        : heap_(&heap)
        , handle_(handle)
    {
        assert(handle.type() == object_type::integer);
    }

    object(object_heap& heap, int64_t value)
        : heap_(&heap)
        , handle_(heap.allocate(object_type::integer, sizeof(int64_t)))
    {
        set_value(value);
    }

    int64_t value() const {
        return heap_->access<int64_t>(handle_);
    }

    void set_value(int64_t value) {
        heap_->access<int64_t>(handle_) = value;
    }

private:
    object_heap*  heap_;
    object_handle handle_;
};

template<>
class object<object_type::symbol> : public object_base {
    using name_t = varchar<16>;
public:
    object(object_heap& heap, const char* first, const char* last)
        : heap_(&heap)
        , handle_(heap.allocate(object_type::symbol, sizeof(name_t)))
    {
        new(&heap_->access<name_t>(handle_)) name_t(first, last-first);
    }

    object(object_heap& heap, object_handle handle)
        : heap_(&heap)
        , handle_(handle)
    {
        assert(handle.type() == object_type::symbol);
    }

    const name_t& name() const {
        return heap_->access<name_t>(handle_);
    }

private:
    object_heap*  heap_;
    object_handle handle_;
};

template<>
class object<object_type::string> : public object_base {
    using len_t = uint16_t;

    struct string {
        len_t len;
        char  buf[1];
    };

public:
    object(object_heap& heap, const char* first, const char* last)
        : heap_(&heap)
    {
        size_t len = last - first;
        if ((std::numeric_limits<len_t>::max() - 1) < len) {
            throw std::runtime_error("string is too long to be stored in heap");
        }

        handle_ = heap_->allocate(object_type::string, sizeof(len_t) + len + 1);

        auto& s = heap_->access<string>(handle_);
        s.len = static_cast<len_t>(len);
        memcpy(s.buf, first, len + 1);
    }

    object(object_heap& heap, object_handle handle)
        : heap_(&heap)
        , handle_(handle)
    {
        assert(handle.type() == object_type::string);
    }

    const char* c_str() const {
        return heap_->access<string>(handle_).buf;
    }

    size_t size() const {
        return heap_->access<string>(handle_).len;
    }

private:
    object_heap*  heap_;
    object_handle handle_;
};

template<>
class object<object_type::cell> : public object_base {
    struct cell {
        // we've got enough extra storage to do interesting things here:
        //   - cache symbol lookups?
        //   - environment/closure?
        object_handle car;
        // object_handle car_tag;
        object_handle cdr;
        // object_handle cdr_tag;
    };
public:
    object(object_heap& heap)
        : heap_(&heap)
        , handle_(heap.allocate(object_type::cell, sizeof(cell)))
    {
    }

    object(object_heap& heap, object_handle car, object_handle cdr)
        : heap_(&heap)
        , handle_(heap.allocate(object_type::cell, sizeof(cell)))
    {
        set_car(car);
        set_cdr(cdr);
    }

    // FIXME: explicit?
    object(object_heap& heap, object_handle handle)
        : heap_(&heap)
        , handle_(handle)
    {
        assert(handle.type() == object_type::cell);
    }

    object_handle car() const {
        return heap_->access<cell>(handle_).car;
    }

    void set_car(object_handle car) {
        heap_->access<cell>(handle_).car = car;
    }

    object_handle cdr() const {
        return heap_->access<cell>(handle_).cdr;
    }

    void set_cdr(object_handle cdr) {
        heap_->access<cell>(handle_).cdr = cdr;
    }

private:
    object_heap*  heap_;
    object_handle handle_;
};

using nil_object = object<object_type::nil>;
using integer_object = object<object_type::integer>;
using symbol_object = object<object_type::symbol>;
using string_object = object<object_type::string>;
using cell = object<object_type::cell>;

template<object_type type>
object<type> object_cast(object_heap& heap, object_handle handle) {
    if (handle.type() != type) {
        throw std::runtime_error("invalid object_cast");
    }

    return object<type>(heap, handle);
}

template<object_type type, typename... Args>
object<type> make_object(object_heap& heap, Args&&... args) {
    return object<type>(heap, std::forward<Args>(args)...);
}
#endif

}
