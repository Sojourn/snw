#include <stdexcept>
#include <type_traits>
#include <cstdint>
#include <cstddef>

template<size_t capacity, typename Result, typename... Args>
class snw::basic_function<capacity, Result(Args...)>::callable {
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

template<size_t capacity, typename Result, typename... Args>
template<typename Fn>
class snw::basic_function<capacity, Result(Args...)>::callable_functor : public callable {
public:
    template<typename F>
    callable_functor(F&& fn)
        : fn_(std::forward<F>(fn))
    {
    }

    bool is_null() const override {
        return false;
    }

    Result apply(Args... args) override {
        return fn_(std::forward<Args>(args)...);
    }

    void move_to(void* target) override {
        new(target) callable_functor<Fn>(std::move(fn_));
    }

private:
    Fn fn_;
};

template<size_t capacity, typename Result, typename... Args>
template<typename T>
class snw::basic_function<capacity, Result(Args...)>::callable_member_function : public callable {
public:
    callable_member_function(T* object, member_function_ptr<T> mem_fn)
        : object_(object)
        , mem_fn_(mem_fn)
    {
    }

    bool is_null() const override {
        return false;
    }

    Result apply(Args... args) override {
        return (object_->*mem_fn_)(std::forward<Args>(args)...);
    }

    void move_to(void* target) override {
        new(target) callable_member_function<T>(object_, mem_fn_);
    }

private:
    T*                     object_;
    member_function_ptr<T> mem_fn_;
};

template<size_t capacity, typename Result, typename... Args>
snw::basic_function<capacity, Result(Args...)>::basic_function() {
    create_callable<callable>();
}

template<size_t capacity, typename Result, typename... Args>
snw::basic_function<capacity, Result(Args...)>::basic_function(basic_function&& other) {
    other.get_callable().move_to(storage_); // TODO: combine virtual dispatches
    other.destroy_callable();
    other.create_callable<callable>();
}

template<size_t capacity, typename Result, typename... Args>
template<typename Fn>
snw::basic_function<capacity, Result(Args...)>::basic_function(Fn&& fn) {
    using callable_impl = callable_functor<typename std::decay<Fn>::type>;

    create_callable<callable_impl>(std::forward<Fn>(fn));
}

template<size_t capacity, typename Result, typename... Args>
template<typename T>
snw::basic_function<capacity, Result(Args...)>::basic_function(T* object, member_function_ptr<T> mem_fn) {
    using callable_impl = callable_member_function<T>;

    create_callable<callable_impl>(object, mem_fn);
}

template<size_t capacity, typename Result, typename... Args>
snw::basic_function<capacity, Result(Args...)>::~basic_function() {
    destroy_callable();
}

template<size_t capacity, typename Result, typename... Args>
auto snw::basic_function<capacity, Result(Args...)>::operator=(basic_function&& rhs) -> basic_function& {
    if (this != &rhs) {
        destroy_callable();
        rhs.get_callable().move_to(storage_); // TODO: combine virtual dispatches
        rhs.destroy_callable();
        rhs.create_callable<callable>();
    }

    return *this;
}

template<size_t capacity, typename Result, typename... Args>
template<typename Fn>
auto snw::basic_function<capacity, Result(Args...)>::operator=(Fn&& fn) -> basic_function& {
    using callable_impl = callable_functor<typename std::decay<Fn>::type>;

    destroy_callable();
    create_callable<callable_impl>(std::forward<Fn>(fn));
}

template<size_t capacity, typename Result, typename... Args>
Result snw::basic_function<capacity, Result(Args...)>::operator()(Args... args) {
    return get_callable().apply(std::forward<Args>(args)...);
}

template<size_t capacity, typename Result, typename... Args>
snw::basic_function<capacity, Result(Args...)>::operator bool() const {
    return !get_callable().is_null();
}

template<size_t capacity, typename Result, typename... Args>
template<typename CallableImpl, typename... Params>
void snw::basic_function<capacity, Result(Args...)>::create_callable(Params&&... params) {
    static_assert(sizeof(CallableImpl) <= capacity, "capacity is too small");
    static_assert(alignof(CallableImpl) <= alignment, "alignment is too small");

    new(storage_) CallableImpl(std::forward<Params>(params)...);
}

template<size_t capacity, typename Result, typename... Args>
void snw::basic_function<capacity, Result(Args...)>::destroy_callable() {
    get_callable().~callable();
}

template<size_t capacity, typename Result, typename... Args>
auto snw::basic_function<capacity, Result(Args...)>::get_callable() -> callable& {
    return *reinterpret_cast<callable*>(storage_);
}

template<size_t capacity, typename Result, typename... Args>
auto snw::basic_function<capacity, Result(Args...)>::get_callable() const -> const callable& {
    return *reinterpret_cast<const callable*>(storage_);
}
