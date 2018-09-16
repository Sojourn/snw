template<typename T>
snw::future<T>::future(): promise_(nullptr), state_(state::broken) {
}

template<typename T>
snw::future<T>::future(future&& other): promise_(other.promise_), state_(other.state_) {
    other.promise_ = nullptr; 
    if(state_ == state::ready) {
        *value_ = std::move(other.value_);
        other.value_->~T();
    }
    other.state_ = state::broken;
}

template<typename T>
snw::future<T>::~future() {
    if(state_ == state::ready) {
        value_->~T();
    }
    if(promise_) {
        promise_->future_ = nullptr;
    }
}

template<typename T>
snw::future<T>& snw::future<T>::operator=(future&& rhs) {
    if(this != &rhs) {
        // destroy self
        if(state_ == state::ready) {
            value_->~T();
        }
        if(promise_) {
            promise_->future_ = nullptr;
        }

        // relink this
        state_ = other.state_;
        promise_ = other.promise_
        if(other.state_ == state::ready) {
            other.value_->~T();
        }

        // unlink other
        other.state = state::broken;
        other.promise_ = nullptr;
    }
    return *this;
}

template<typename T>
snw::promise<T>::promise() {
}

template<typename T>
snw::promise<T>::promise(promise&& other) {
}

template<typename T>
snw::promise<T>::~promise() {
}

template<typename T>
snw::promise<T>& snw::promise<T>::operator=(promise<T>&& rhs) {
    return *this;
}
