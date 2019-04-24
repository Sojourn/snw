#include <iostream>
#include <chrono>
#include <vector>
#include <limits>
#include <string>
#include <atomic>
#include <memory>
#include <thread>
#include <stdexcept>
#include <type_traits>
#include <cstdio>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cassert>

#include <unistd.h>

#include "snw_stream.h"
#include "align.h"
#include "varchar.h"
#include "intrusive_list.h"

namespace snw {

#if __cpp_lib_is_invocable
    template<class F, class... Args>
    using invoke_result = std::invoke_result<F, Args...>;
#else
    template<class F, class... Args>
    using invoke_result = std::result_of<F(Args...)>;
#endif

template<size_t capacity, typename>
class basic_function;

template<size_t capacity, typename Result, typename... Args>
class basic_function<capacity, Result(Args...)> {
    static constexpr size_t alignment = alignof(void*);

    template<typename T>
    using member_function_ptr = Result(T::*)(Args...);
public:
    basic_function() {
        create_callable<callable>();
    }

    basic_function(basic_function&& other) {
        other.get_callable().move_to(storage_);
        other.destroy_callable();
        other.create_callable<callable>();
    }

    basic_function(const basic_function&) = delete;

    template<typename Fn>
    basic_function(Fn&& fn) {
        using callable_impl = functor<typename std::decay<Fn>::type>;

        create_callable<callable_impl>(std::forward<Fn>(fn));
    }

    template<typename T>
    basic_function(T* object, member_function_ptr<T> mem_fn) {
        using callable_impl = member_function<T>;

        create_callable<callable_impl>(object, mem_fn);
    }

    ~basic_function() {
        destroy_callable();
    }

    basic_function& operator=(basic_function&& rhs) {
        if (this != &rhs) {
            destroy_callable();
            rhs.get_callable().move_to(storage_);
            rhs.destroy_callable();
            rhs.create_callable<callable>();
        }

        return *this;
    }

    basic_function& operator=(const basic_function&) = delete;

    // FIXME: this breaks operator=(const basic_function&) = delete
    template<typename Fn>
    basic_function& operator=(Fn&& fn) {
        using callable_impl = functor<typename std::decay<Fn>::type>;

        destroy_callable();
        create_callable<callable_impl>(std::forward<Fn>(fn));
    }

    explicit operator bool() const {
        return !get_callable().is_null();
    }

    template<typename... Args_>
    Result operator()(Args_&&... args) {
        return get_callable().apply(std::forward<Args_>(args)...);
    }

private:
    class callable {
    public:
        virtual ~callable() = default;

        virtual bool is_null() const {
            return true;
        }

        virtual Result apply(Args... args) {
            throw std::runtime_error("function is empty");
        }

        virtual void move_to(void* target) {
            new(target) callable;
        }
    };

    template<typename Fn>
    class functor : public callable {
    public:
        template<typename F>
        functor(F&& fn)
            : fn_(std::move(fn))
        {
        }

        bool is_null() const override {
            return false;
        }

        Result apply(Args... args) override {
            return fn_(args...);
        }

        void move_to(void* target) override {
            new(target) functor<Fn>(std::move(fn_));
        }

    private:
        Fn fn_;
    };

    template<typename T>
    class member_function : public callable {
    public:
        member_function(T* object, member_function_ptr<T> mem_fn)
            : object_(object)
            , mem_fn_(mem_fn)
        {
        }

        bool is_null() const override {
            return false;
        }

        Result apply(Args... args) override {
            return (object_->*mem_fn_)(args...);
        }

        void move_to(void* target) override {
            new(target) member_function<T>(object_, mem_fn_);
        }

    private:
        T*                     object_;
        member_function_ptr<T> mem_fn_;
    };

    callable& get_callable() {
        return *reinterpret_cast<callable*>(storage_);
    }

    template<typename Callable, typename... Params>
    void create_callable(Params&&... params) {
        static_assert(sizeof(Callable) <= capacity, "capacity is too small");
        static_assert(alignof(Callable) <= alignment, "alignment is too small");

        new(storage_) Callable(std::forward<Params>(params)...);
    }

    void destroy_callable() {
        get_callable().~callable();
    }

private:
    alignas(alignment) uint8_t storage_[capacity];
};

}

struct copyable_arg {
};

struct movable_arg {
    movable_arg() = default;
    movable_arg(movable_arg&&) = default;
    movable_arg(const movable_arg&) = delete;
    ~movable_arg() = default;
    movable_arg& operator=(movable_arg&&) = default;
    movable_arg& operator=(const movable_arg&) = delete;
};

struct foo {
    char a() {
        return 'a';
    }

    char b(copyable_arg) {
        return 'b';
    }

    char c(movable_arg) {
    }
};

int main(int argc, const char** argv) {
    foo f;
    copyable_arg carg;
    movable_arg marg;

    {
        snw::basic_function<64, char()> fn(&f, &foo::a);
        std::cout << fn() << std::endl;
    }
    {
        snw::basic_function<64, char(copyable_arg)> fn(&f, &foo::b);
        std::cout << fn(carg) << std::endl;
    }
    // {
    //     snw::basic_function<64, char(movable_arg)> fn(&f, &foo::c);
    //     std::cout << fn(std::move(marg)) << std::endl;
    // }

    return 0;
}
