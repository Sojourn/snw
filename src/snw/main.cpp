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
public:
    basic_function() {
        static_assert(sizeof(callable) <= capacity, "capacity is too small");
        new(storage_) callable;
    }

    basic_function(basic_function&& other) {
        other.get_callable().move_to(storage_);
        other.get_callable().~callable();

        static_assert(sizeof(callable) <= capacity, "capacity is too small");
        static_assert(alignof(callable) <= alignment, "alignment is too small");
        new(other.storage_) callable;
    }

    basic_function(const basic_function&) = delete;

    template<typename Fn>
    basic_function(Fn&& fn) {
        using functor_t = functor<typename std::decay<Fn>::type>;
        static_assert(sizeof(functor_t) <= capacity, "capacity is too small");
        new(storage_) functor_t(std::forward<Fn>(fn));
    }

    ~basic_function() {
        get_callable().~callable();
    }

    basic_function& operator=(basic_function&& rhs) {
        if (this != &rhs) {
            get_callable().~callable();
            rhs.get_callable().move_to(storage_);

            static_assert(sizeof(callable) <= capacity, "callable is too small");
            new(rhs.storage_) callable;
        }

        return *this;
    }

    basic_function& operator=(const basic_function&) = delete;

    // FIXME: this breaks operator=(const basic_function&) = delete
    template<typename Fn>
    basic_function& operator=(Fn&& fn) {
        get_callable().~callable();

        using functor_t = functor<typename std::decay<Fn>::type>;
        static_assert(sizeof(functor_t) <= capacity, "capacity is too small");
        new(storage_) functor_t(std::forward<Fn>(fn));
    }

    explicit operator bool() const {
        return !get_callable().is_empty();
    }

    template<typename... Args_>
    Result operator()(Args_&&... args) {
        return get_callable().apply(std::forward<Args_>(args)...);
    }

private:
    class callable {
    public:
        virtual ~callable() = default;

        virtual bool is_empty() const {
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

        bool is_empty() const override {
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

    callable& get_callable() {
        return *reinterpret_cast<callable*>(storage_);
    }

private:
    alignas(alignment) uint8_t storage_[capacity];
};

}

int main(int argc, const char** argv) {
    int value = 13;

    snw::basic_function<64, int()> fn([=]() mutable {
        return value;
    });

    snw::basic_function<64, int()> fn2;

    fn2 = std::move(fn);

    std::cout << fn2() << std::endl;
    try {
        fn();
        abort();
    }
    catch (const std::exception& ex) {
        std::cout << ex.what() << std::endl;
    }

    return 0;
}
