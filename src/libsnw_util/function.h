#pragma once

#include <cstdint>
#include <cstddef>

namespace snw {

template<size_t capacity, typename>
class basic_function;

template<size_t capacity, typename Result, typename... Args>
class basic_function<capacity, Result(Args...)> {
    static constexpr size_t alignment = alignof(void*);

    template<typename T>
    using member_function_ptr = Result(T::*)(Args...);
public:
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

    explicit operator bool() const;

    template<typename... Args_>
    Result operator()(Args_&&... args);

private:
    class callable {
    public:
        virtual ~callable() = default;

        virtual bool is_null() const {
            return true;
        }

        virtual Result apply(Args...) {
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
            return fn_(std::move(args)...);
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
            return (object_->*mem_fn_)(std::move(args)...);
        }

        void move_to(void* target) override {
            new(target) member_function<T>(object_, mem_fn_);
        }

    private:
        T*                     object_;
        member_function_ptr<T> mem_fn_;
    };


    template<typename Callable, typename... Params>
    void create_callable(Params&&... params);
    void destroy_callable();
    callable& get_callable();

private:
    alignas(alignment) uint8_t storage_[capacity];
};

}

#include "function.hpp"
