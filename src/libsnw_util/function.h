#pragma once

#include <cstdint>
#include <cstddef>

namespace snw {

// A non-allocating, movable, and mutable-only function implementation
template<size_t capacity, typename>
class basic_function;

template<size_t capacity, typename Result, typename... Args>
class basic_function<capacity, Result(Args...)> {
    static constexpr size_t alignment = alignof(void*);

    template<typename T>
    using member_function_ptr = Result(T::*)(Args...);
public:
    using result_type = Result;

    basic_function();
    basic_function(basic_function&& other);
    basic_function(const basic_function&) = delete;

    template<typename Fn>
    basic_function(Fn&& fn);

    template<typename T>
    basic_function(T* object, member_function_ptr<T> mem_fn);

    ~basic_function();

    basic_function& operator=(basic_function&& rhs);
    basic_function& operator=(const basic_function&) = delete;

    // FIXME: this breaks operator=(const basic_function&) = delete
    //        still get a compile time error thought
    template<typename Fn>
    basic_function& operator=(Fn&& fn);

    template<typename... Args_>
    Result operator()(Args_&&... args);

    explicit operator bool() const;

private:
    class callable;

    template<typename Fn>
    class callable_functor;

    template<typename T>
    class callable_member_function;

    template<typename Callable, typename... Params>
    void create_callable(Params&&... params);
    void destroy_callable();
    callable& get_callable();

private:
    alignas(alignment) uint8_t storage_[capacity];
};

// This should be large enough for most use cases. It is probably ok to define a
// bespoke_function if there is a case where a larger closure capacity is useful.
#if 0
// TODO: check if we can use this in a future version of c++
template<typename T, typename... Args>
using function = basic_function<64, T(Args...)>;
#else
template<typename Signature>
using function = basic_function<64, Signature>;
#endif

}

#include "function.hpp"
