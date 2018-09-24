#include <cassert>

template<typename T>
snw::future<T>::future()
    : promise_(nullptr)
    , state_(state::broken)
{
}

template<typename T>
snw::future<T>::future(promise<T>* promise)
    : promise_(promise)
    , state_(state::waiting)
{
}

template<typename T>
snw::future<T>::future(future&& other)
    : promise_(other.promise_)
    , state_(other.state_)
{
    if(promise_) {
        promise_->future_ = this;
    }

    if(state_ == state::ready) {
        value_.create(std::move(*other.value_));
        other.value_.destroy();
    }

    other.promise_ = nullptr;
    other.state_ = state::broken;
}

template<typename T>
snw::future<T>::~future() {
    if(promise_) {
        promise_->future_ = nullptr;
    }

    if(state_ == state::ready) {
        value_.destroy();
    }
}

template<typename T>
snw::future<T>& snw::future<T>::operator=(future&& rhs) {
    if(this != &rhs) {
        // unlink
        if(promise_) {
            promise_->future_ = nullptr;
        }

        // clear value
        if(state_ == state::ready) {
            value_.destroy();
        }

        // relink
        if((promise_ = rhs.promise_)) {
            promise_->future_ = this;
        }
        rhs.promise_ = nullptr;

        // take value
        if((state_ = rhs.state_) == state::ready) {
            value_.create(std::move(*rhs.value_));
            rhs.value_.destroy();
        }
        rhs.state_ = state::broken;
    }
    return *this;
}

template<typename T>
bool snw::future<T>::is_broken() const {
    return state_ == state::broken;
}

template<typename T>
bool snw::future<T>::is_waiting() const {
    return state_ == state::waiting;
}

template<typename T>
bool snw::future<T>::has_value() const {
    return state_ == state::ready;
}

template<typename T>
T& snw::future<T>::value() {
    assert(has_value());
    return *value_;
}

template<typename T>
const T& snw::future<T>::value() const {
    assert(has_value());
    return *value_;
}

template<typename T>
snw::promise<T>::promise()
    : future_(nullptr)
{
}

template<typename T>
snw::promise<T>::promise(future<T>* future)
    : future_(future)
{
}

template<typename T>
snw::promise<T>::promise(promise&& other)
    : future_(other.future_)
{
    if(future_) {
        other.future_ = nullptr;
        future_->promise_ = this;
    }
}

template<typename T>
snw::promise<T>::~promise() {
    if(future_) {
        future_->promise_ = nullptr;
    }
}

template<typename T>
snw::promise<T>& snw::promise<T>::operator=(promise<T>&& rhs) {
    if(this != &rhs) {
        // unlink
        if(future_) {
            future_->promise_ = nullptr;
        }

        // relink
        if((future_ = rhs.future_)) {
            rhs.future_ = nullptr;
            future_->promise_ = this;
        }
    }
    return *this;
}

template<typename T>
void snw::promise<T>::set_value(T value) {
    if(future_) {
        assert(!future_->has_value());
        future_->value_.create(std::move(value));
        future_->state_ = future<T>::state::ready;
    }
}
