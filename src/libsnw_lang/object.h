#pragma once

#include <string>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cassert>
#include "object_handle.h"
#include "object_heap.h"

namespace snw {

using symbol = varchar<16>;

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

public:

private:
    static constexpr uint16_t addr_ = 0;

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

public:
    bool value() const {
        return heap().access<bool>(*this);
    }

    void set_value(bool value) {
        heap().access<bool>(*this) = value;
    }

private:
    static object create(object_heap& heap, bool value) {
        object result(heap.allocate(object_type::boolean, sizeof(value)));
        result.set_value(value);
        return result;
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
    template<object_type type>
    friend object<type> object_cast(object_handle);

    template<object_type type>
    friend object<type> unsafe_object_cast(object_handle);

    template<object_type type, typename... Args>
    friend object<type> make_object(object_heap&, Args&&...);

public:
    const symbol& value() const {
        return heap().access<symbol>(*this);
    }

    void set_value(const symbol& value) {
        heap().access<symbol>(*this) = value;
    }

    void set_value(const char* value) {
        set_value(symbol(value));
    }

    void set_value(const char* first, const char* last) {
        set_value(symbol(first, last - first));
    }

private:
    static object create(object_heap& heap, const char* first, const char* last) {
        object result(heap.allocate(object_type::symbol, sizeof(symbol)));
        result.set_value(first, last);
        return result;
    }

    static object create(object_heap& heap, const char* name) {
        return create(heap, name, name+ strlen(name));
    }
    

    object(object_handle handle)
        : object_handle(std::move(handle))
    {
    }
};

template<>
class object<object_type::string> : public object_handle {
    template<object_type type>
    friend object<type> object_cast(object_handle);

    template<object_type type>
    friend object<type> unsafe_object_cast(object_handle);

    template<object_type type, typename... Args>
    friend object<type> make_object(object_heap&, Args&&...);

    struct string {
        uint16_t len;
        // uint16_t cap;
        char     buf[0];
    };
    static_assert(sizeof(string) == sizeof(uint16_t), "Compiler extension not available");

public:
    std::string str() const {
        return std::string(c_str(), size());
    }

    const char* c_str() const {
        return begin();
    }

    const char* begin() const {
        return heap().access<string>(*this).buf;
    }

    const char* end() const {
        return begin() + size();
    }

    size_t size() const {
        return heap().access<string>(*this).len;
    }

    // be careful.
    void set_value(const char* first, const char* last) {
        size_t len = last - first;

        auto& str = heap().access<string>(*this);
        str.len = len;
        memcpy(str.buf, first, len + 1);
    }

private:
    static object create(object_heap& heap, const char* first, const char* last) {
        size_t len = last - first;
        object result(heap.allocate(object_type::string, sizeof(string) + len + 1));
        result.set_value(first, last);
        return result;
    }

    static object create(object_heap& heap, const char* str) {
        return create(heap, str, str + strlen(str));
    }

    object(object_handle handle)
        : object_handle(std::move(handle))
    {
    }
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
    object_handle car() /* const */ {
        return object_handle(heap(), heap().access<cell>(*this).car);
    }

    void set_car(const object_handle& handle) {
        heap().access<cell>(*this).car = handle.raw_handle();
    }

    object_handle cdl() /* const */ {
        return object_handle(heap(), heap().access<cell>(*this).cdl);
    }

    void set_cdl(const object_handle& handle) {
        heap().access<cell>(*this).cdl = handle.raw_handle();
    }

private:
    static object create(object_heap& heap) {
        return object(heap.allocate(object_type::cell, sizeof(cell)));
    }

    object(object_handle handle)
        : object_handle(std::move(handle))
    {
    }
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

}
