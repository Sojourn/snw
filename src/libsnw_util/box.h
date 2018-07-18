#pragma once

#include <type_traits>
#include <utility>

namespace snw {

template <typename T>
class box {
public:
    constexpr box() = default;

    box(box&&) = delete;
    box(const box&) = delete;
    box &operator=(box&&) = delete;
    box &operator=(const box&) = delete;

    template <typename... Args>
    void create(Args&&... args);
    T release();
    void destroy();

    T& operator*();
    const T& operator*() const;
    T* operator->();
    const T* operator->() const;

private:
    using storage = typename std::aligned_storage<sizeof(T), alignof(T)>::type;

    storage storage_;
};

#include "box.hpp"

}
