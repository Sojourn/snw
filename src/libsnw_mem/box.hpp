#include "box.h"

template <typename T>
template <typename... Args>
void snw::box<T>::create(Args&&... args)
{
    new(&storage_) T{ std::forward<Args>(args)... };
}

template <typename T>
T snw::box<T>::release()
{
    T result = std::move(operator*());
    destroy();
    return result;
}

template <typename T>
void snw::box<T>::destroy()
{
    (operator*()).~T();
}

template <typename T>
T& snw::box<T>::operator*()
{
    return *(operator->());
}

template <typename T>
const T& snw::box<T>::operator*() const
{
    return *(operator->());
}

template <typename T>
T* snw::box<T>::operator->()
{
    return reinterpret_cast<T*>(&storage_);
}

template <typename T>
const T* snw::box<T>::operator->() const
{
    return reinterpret_cast<const T*>(&storage_);
}
