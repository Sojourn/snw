#pragma once

#include <cassert>

template<typename T, size_t capacity_>
snw::array<T, capacity_>::array()
    : size_(0)
{
}

template<typename T, size_t capacity_>
snw::array<T, capacity_>::array(array&& other)
{
    for(size_t i = 0; i < other.size_; ++i) {
        data_[i].create(other.data_[i].release());
    }

    size_ = other.size_;
    other.size_ = 0;
}

template<typename T, size_t capacity_>
snw::array<T, capacity_>::~array()
{
    clear();
}

template<typename T, size_t capacity_>
snw::array<T, capacity_>& snw::array<T, capacity_>::operator=(array&& rhs)
{
    if(this != &rhs) {
        clear();

        for(size_t i = 0; i < rhs.size_; ++i) {
            data_[i].create(rhs.data_[i].release());
        }

        size_ = rhs.size_;
        rhs.size_ = 0;
    }

    return *this;
}

template<typename T, size_t capacity_>
size_t snw::array<T, capacity_>::size() const
{
    return size_;
}

template<typename T, size_t capacity_>
size_t snw::array<T, capacity_>::capacity() const
{
    return capacity_;
}

template<typename T, size_t capacity_>
bool snw::array<T, capacity_>::empty() const
{
    return (size_ == 0);
}

template<typename T, size_t capacity_>
bool snw::array<T, capacity_>::full() const
{
    return (size_ == capacity_);
}

template<typename T, size_t capacity_>
typename snw::array<T, capacity_>::iterator snw::array<T, capacity_>::begin()
{
    return data_[0].operator->();
}

template<typename T, size_t capacity_>
typename snw::array<T, capacity_>::iterator snw::array<T, capacity_>::end()
{
    return data_[size_].operator->();
}

template<typename T, size_t capacity_>
typename snw::array<T, capacity_>::const_iterator snw::array<T, capacity_>::begin() const
{
    return data_[0].operator->();
}

template<typename T, size_t capacity_>
typename snw::array<T, capacity_>::const_iterator snw::array<T, capacity_>::end() const
{
    return data_[size_].operator->();
}

template<typename T, size_t capacity_>
T* snw::array<T, capacity_>::data()
{
    return begin();
}

template<typename T, size_t capacity_>
const T* snw::array<T, capacity_>::data() const
{
    return begin();
}

template<typename T, size_t capacity_>
T& snw::array<T, capacity_>::front()
{
    assert(!empty());
    return *begin();
}

template<typename T, size_t capacity_>
const T& snw::array<T, capacity_>::front() const
{
    assert(!empty());
    return *begin();
}

template<typename T, size_t capacity_>
T& snw::array<T, capacity_>::back()
{
    assert(!empty());
    return *(end() - 1);
}

template<typename T, size_t capacity_>
const T& snw::array<T, capacity_>::back() const
{
    assert(!empty());
    return *(end() - 1);
}

template<typename T, size_t capacity_>
T& snw::array<T, capacity_>::operator[](size_t index)
{
    assert(index < size_);
    return *data_[index];
}

template<typename T, size_t capacity_>
const T& snw::array<T, capacity_>::operator[](size_t index) const
{
    assert(index < size_);
    return *data_[index];
}

template<typename T, size_t capacity_>
bool snw::array<T, capacity_>::push_back(T value)
{
    if(size_ == capacity_) {
        return false;
    }

    data_[size_++].create(value);
    return true;
}

template<typename T, size_t capacity_>
bool snw::array<T, capacity_>::insert(const_iterator pos, T value)
{
    if(size_ == capacity_) {
        return false;
    }

    // shift existing values up
    size_t off = pos - begin();
    for(size_t i = size_; i > off; --i) {
        data_[i].create(data_[i - 1].release());
    }

    data_[off].create(value);
	size_++;
    return true;
}

template<typename T, size_t capacity_>
void snw::array<T, capacity_>::pop_back()
{
    if(size_ == 0) {
        return;
    }

    data_[--size_].destroy();
}

template<typename T, size_t capacity_>
void snw::array<T, capacity_>::erase(const_iterator pos)
{
    size_t off = pos - begin();
    if(off == size_) {
        return; // pos == end
    }

    data_[off].destroy();

    // shift remaining values down
    for(size_t i = off; i < (size_ - 1); ++i) {
        data_[i].create(data_[i + 1].release());
    }

	size_--;
}

template<typename T, size_t capacity_>
void snw::array<T, capacity_>::clear()
{
    for(size_t i = 0; i < size_; ++i) {
        data_[i].destroy();
    }

    size_ = 0;
}
