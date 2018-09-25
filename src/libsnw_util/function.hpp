template<typename Result, typename... Args>
snw::function<Result(Args...)>::function() {
}

template<typename Result, typename... Args>
snw::function<Result(Args...)>::function(function&& other)
    : vtable_(other.vtable_)
{
    if(vtable_) {
        vtable_->move(&storage_, &other.storage_);
        other.vtable_->destroy(&other.storage_);
        other.vtable_ = nullptr;
    }
}

template<typename Result, typename... Args>
snw::function<Result(Args...)>::function(Result(*fn)(Args...))
    : vtable_(nullptr)
{
#if 0
    struct storage_impl {
        Result(*fn)(Args...);
    };
    static_assert(sizeof(storage_impl) <= sizeof(storage_), )

    static const vtable *vtable_impl = {
        [](void* raw_self) {
            auto self = reinterpret_cast<storage_impl*>(raw_self);
        },
        [](void* raw_self) {
            auto self = reinterpret_cast<storage_impl*>(raw_self);
        },
        [](void* raw_self, void* raw_other) {
            auto self = reinterpret_cast<storage_impl*>(raw_self);
            auto other = reinterpret_cast<storage_impl*>(raw_other);
        }
    };

    vtable_ = &vtable_impl;
    auto self = reinterpret_cast<storage_impl*>(storage_);
    self->fn = fn;
#endif
}

template<typename Result, typename... Args>
snw::function<Result(Args...)>::~function() {
}
