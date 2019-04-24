#include <stdexcept>
#include <type_traits>
#include <cstdint>
#include <cstddef>

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
template<typename... Args_>
Result snw::basic_function<capacity, Result(Args...)>::operator()(Args_&&... args) {
    return get_callable().apply(std::forward<Args_>(args)...);
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
