#pragma once

#include <cstddef>
#include "types.h"
#include "box.h"

namespace snw {

template<typename T, size_t capacity_>
class array {
public:
    using value_type = T;
    using iterator = T*;
    using const_iterator = const T*;

public:
    array();
    array(array&& other);
    array(const array&) = delete;
    ~array();

    array& operator=(array&& rhs);
    array& operator=(const array&) = delete;

    size_t size() const;
    size_t capacity() const;
    bool empty() const;
    bool full() const;

    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;

    T& front();
    const T& front() const;
    T& back();
    const T& back() const;

    T& operator[](size_t index);
    const T& operator[](size_t index) const;

    bool push_back(T value);
    bool insert(const_iterator pos, T value);
    void pop_back();
    void erase(const_iterator pos);
    void clear();

private:
    size_t size_; // TODO: the type of size_ should depend on alignof(T) and capacity_
    box<T> data_[capacity_];
};

}

#include "array.hpp"
