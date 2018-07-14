#pragma once

#include <type_traits>
#include <iterator>
#include <cassert>
#include "types.h"

namespace snw {

class intrusive_list_node {
public:
    intrusive_list_node();
    intrusive_list_node(intrusive_list_node&& other);
    intrusive_list_node(const intrusive_list_node&) = delete;

    ~intrusive_list_node();

    intrusive_list_node& operator=(intrusive_list_node&& rhs);
    intrusive_list_node& operator=(const intrusive_list_node&) = delete;

    bool is_linked() const;
    void unlink();

    // prepend this to next
    void link(intrusive_list_node* next);

    // next = prev = this
    void self_link();

    intrusive_list_node* next();
    const intrusive_list_node* next() const;
    intrusive_list_node* prev();
    const intrusive_list_node* prev() const;

private:
    intrusive_list_node* next_;
    intrusive_list_node* prev_;
};

// TODO: make iterators member classes of intrusive_list
template <typename T, intrusive_list_node T::*member_node>
class intrusive_list_const_iterator {
public:
    using const_iterator = intrusive_list_const_iterator;

    using value_type = T;
    using pointer = const T*;
    using reference = const T&;
    using iterator_category = std::bidirectional_iterator_tag;

    intrusive_list_const_iterator(const intrusive_list_node* node = nullptr);

    reference operator*() const;
    pointer operator->() const;

    const_iterator& operator=(const const_iterator&) = default;
    const_iterator& operator++(); // prefix increment
    const_iterator operator++(int); // postfix increment
    const_iterator& operator--(); // prefix decrement
    const_iterator operator--(int); // postfix decrement

    bool operator==(const const_iterator& rhs) const;
    bool operator!=(const const_iterator& rhs) const;

    const intrusive_list_node* node() const;

protected:
    intrusive_list_const_iterator(intrusive_list_node* node);

    intrusive_list_node* node_;
};

// TODO: make iterators member classes of intrusive_list
template <typename T, intrusive_list_node T::*member_node>
class intrusive_list_iterator : public intrusive_list_const_iterator<T, member_node> {
public:
    using iterator = intrusive_list_iterator;

    using value_type = typename std::remove_cv<T>::type;
    using pointer = T*;
    using reference = T&;
    using iterator_category = std::bidirectional_iterator_tag;

    intrusive_list_iterator(intrusive_list_node* node = nullptr);

    reference operator*() const;
    pointer operator->() const;

    iterator& operator=(const iterator&) = default;
    iterator& operator++(); // prefix increment
    iterator operator++(int); // postfix increment
    iterator& operator--(); // prefix decrement
    iterator operator--(int); // postfix decrement

    intrusive_list_node* node();
};

template <typename T, intrusive_list_node T::*node>
class intrusive_list {
public:
    using const_iterator = intrusive_list_const_iterator<T, node>;
    using iterator = intrusive_list_iterator<T, node>;

    intrusive_list();
    intrusive_list(intrusive_list&& other);
    intrusive_list(const intrusive_list&) = delete;

    ~intrusive_list();

    intrusive_list& operator=(intrusive_list&& rhs);
    intrusive_list& operator=(const intrusive_list& rhs) = delete;

    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;

    bool empty() const;

    T& front();
    const T& front() const;
    T& back();
    const T& back() const;

    iterator insert(const_iterator pos, T& value);
    iterator push_back(T& value);
    iterator push_front(T& value);
    iterator erase(const_iterator pos);
    void pop_front();
    void pop_back();
    void clear();

private:
    intrusive_list_node root_;
};

#include "intrusive_list.hpp"

}
