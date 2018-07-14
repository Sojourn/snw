namespace detail {

// TODO: inline / templated variable
template <typename T, intrusive_list_node T::*node_member>
inline size_t intrusive_list_node_offset()
{
    const T* value = nullptr;
    const intrusive_list_node* node = &(value->*node_member);

    auto value_ptr = reinterpret_cast<const uint8_t*>(value);
    auto node_ptr = reinterpret_cast<const uint8_t*>(node);
    assert(value_ptr <= node_ptr && "list node offset should be positive");
    return node_ptr - value_ptr;
}

template <typename T, intrusive_list_node T::*node_member>
inline intrusive_list_node *to_intrusive_list_node(T* value)
{
    auto value_ptr = reinterpret_cast<uint8_t*>(value);
    auto node_ptr = value_ptr + intrusive_list_node_offset<T, node_member>();
    return reinterpret_cast<intrusive_list_node*>(node_ptr);
}

template <typename T, intrusive_list_node T::*node_member>
inline const intrusive_list_node *to_intrusive_list_node(const T* value)
{
    auto value_ptr = reinterpret_cast<const uint8_t*>(value);
    auto node_ptr = value_ptr + intrusive_list_node_offset<T, node_member>();
    return reinterpret_cast<const intrusive_list_node*>(node_ptr);
}

template <typename T, intrusive_list_node T::*node_member>
inline T* from_intrusive_list_node(intrusive_list_node* node)
{
    auto node_ptr = reinterpret_cast<uint8_t*>(node);
    auto value_ptr = node_ptr - intrusive_list_node_offset<T, node_member>();
    return reinterpret_cast<T*>(value_ptr);
}

template <typename T, intrusive_list_node T::*node_member>
inline const T* from_intrusive_list_node(const intrusive_list_node* node)
{
    auto node_ptr = reinterpret_cast<const uint8_t*>(node);
    auto value_ptr = node_ptr - intrusive_list_node_offset<T, node_member>();
    return reinterpret_cast<const T*>(value_ptr);
}

}

inline intrusive_list_node::intrusive_list_node()
    : next_{nullptr}
    , prev_{nullptr}
{
}

inline intrusive_list_node::intrusive_list_node(intrusive_list_node&& other)
    : next_{other.next_}
    , prev_{other.prev_}
{
    other.prev_ = nullptr;
    other.next_ = nullptr;
}

inline intrusive_list_node::~intrusive_list_node()
{
    unlink();
}

inline intrusive_list_node& intrusive_list_node::operator=(intrusive_list_node&& rhs)
{
    if(this != &rhs) {
        unlink();

        next_ = rhs.next_;
        prev_ = rhs.prev_;

        rhs.next_ = nullptr;
        rhs.prev_ = nullptr;
    }

    return *this;
}

inline bool intrusive_list_node::is_linked() const
{
    return next_ != nullptr;
}

inline void intrusive_list_node::unlink()
{
    if(!is_linked()) {
        return;
    }

    auto next = next_;
    auto prev = prev_;

    next_->prev_ = prev;
    prev_->next_ = next;

    next_ = nullptr;
    prev_ = nullptr;
}

inline void intrusive_list_node::link(intrusive_list_node* next)
{
    unlink();

    auto prev = next->prev_;

    next_ = next;
    prev_ = prev;

    next->prev_ = this;
    prev->next_ = this;
}

inline void intrusive_list_node::self_link()
{
    unlink();

    next_ = this;
    prev_ = this;
}

inline intrusive_list_node* intrusive_list_node::next()
{
    return next_;
}

inline const intrusive_list_node* intrusive_list_node::next() const
{
    return next_;
}

inline intrusive_list_node* intrusive_list_node::prev()
{
    return prev_;
}

inline const intrusive_list_node* intrusive_list_node::prev() const
{
    return prev_;
}

template <typename T, intrusive_list_node T::*member_node>
intrusive_list_const_iterator<T, member_node>::intrusive_list_const_iterator(const intrusive_list_node* node)
    : node_{const_cast<intrusive_list_node*>(node)}
        {}

template <typename T, intrusive_list_node T::*member_node>
intrusive_list_const_iterator<T, member_node>::intrusive_list_const_iterator(intrusive_list_node* node)
    : node_{node}
        {}

template <typename T, intrusive_list_node T::*member_node>
auto intrusive_list_const_iterator<T, member_node>::operator*() const -> reference
{
    return *detail::from_intrusive_list_node<T, member_node>(node_);
}

template <typename T, intrusive_list_node T::*member_node>
auto intrusive_list_const_iterator<T, member_node>::operator->() const -> pointer
{
    return detail::from_intrusive_list_node<T, member_node>(node_);
}

template <typename T, intrusive_list_node T::*member_node>
auto intrusive_list_const_iterator<T, member_node>::operator++() -> const_iterator&
{
    node_ = node_->next();
    return *this;
}

template <typename T, intrusive_list_node T::*member_node>
auto intrusive_list_const_iterator<T, member_node>::operator++(int) -> const_iterator
{
    auto result = *this;
    node_ = node_->next();
    return result;
}

template <typename T, intrusive_list_node T::*member_node>
auto intrusive_list_const_iterator<T, member_node>::operator--() -> const_iterator&
{
    node_ = node_->prev();
    return *this;
}

template <typename T, intrusive_list_node T::*member_node>
auto intrusive_list_const_iterator<T, member_node>::operator--(int) -> const_iterator
{
    auto result = *this;
    node_ = node_->prev();
    return result;
}

template <typename T, intrusive_list_node T::*member_node>
bool intrusive_list_const_iterator<T, member_node>::operator==(const const_iterator& rhs) const
{
    return node_ == rhs.node_;
}

template <typename T, intrusive_list_node T::*member_node>
bool intrusive_list_const_iterator<T, member_node>::operator!=(const const_iterator& rhs) const
{
    return node_ != rhs.node_;
}

template <typename T, intrusive_list_node T::*member_node>
const intrusive_list_node* intrusive_list_const_iterator<T, member_node>::node() const
{
    return node_;
}

template <typename T, intrusive_list_node T::*member_node>
intrusive_list_iterator<T, member_node>::intrusive_list_iterator(intrusive_list_node* node)
    : intrusive_list_const_iterator<T, member_node>(node)
        {}

template <typename T, intrusive_list_node T::*member_node>
auto intrusive_list_iterator<T, member_node>::operator*() const -> reference
{
    return *detail::from_intrusive_list_node<T, member_node>(this->node_);
}

template <typename T, intrusive_list_node T::*member_node>
auto intrusive_list_iterator<T, member_node>::operator->() const -> pointer
{
    return detail::from_intrusive_list_node<T, member_node>(this->node_);
}

template <typename T, intrusive_list_node T::*member_node>
auto intrusive_list_iterator<T, member_node>::operator++() -> iterator&
{
    this->node_ = this->node_->next();
    return *this;
}

template <typename T, intrusive_list_node T::*member_node>
auto intrusive_list_iterator<T, member_node>::operator++(int) -> iterator
{
    auto result = *this;
    this->node_ = this->node_->next();
    return result;
}

template <typename T, intrusive_list_node T::*member_node>
auto intrusive_list_iterator<T, member_node>::operator--() -> iterator&
{
    this->node_ = this->node_->prev();
    return *this;
}

template <typename T, intrusive_list_node T::*member_node>
auto intrusive_list_iterator<T, member_node>::operator--(int) -> iterator
{
    auto result = *this;
    this->node_ = this->node_->prev();
    return result;
}

template <typename T, intrusive_list_node T::*member_node>
intrusive_list_node* intrusive_list_iterator<T, member_node>::node()
{
    return this->node_;
}

template <typename T, intrusive_list_node T::*member_node>
intrusive_list<T, member_node>::intrusive_list()
{
    root_.self_link();
}

template <typename T, intrusive_list_node T::*member_node>
intrusive_list<T, member_node>::intrusive_list(intrusive_list&& other)
{
    // insert ourselves before the rhs root
    root_.link(&other.root_);

    // unlink rhs root and make it part of an empty list
    other.root_.self_link();
}

template <typename T, intrusive_list_node T::*member_node>
intrusive_list<T, member_node>::~intrusive_list()
{
    clear();
}

template <typename T, intrusive_list_node T::*member_node>
intrusive_list<T, member_node>& intrusive_list<T, member_node>::operator=(intrusive_list&& rhs)
{
    if(this != &rhs) {
        clear();

        // insert ourselves before the rhs root
        root_.link(&rhs.root_);

        // unlink rhs root and make it part of an empty list
        rhs.root_.self_link();
    }

    return *this;
}

template <typename T, intrusive_list_node T::*member_node>
auto intrusive_list<T, member_node>::begin() -> iterator
{
    return iterator{root_.next()};
}

template <typename T, intrusive_list_node T::*member_node>
auto intrusive_list<T, member_node>::end() -> iterator
{
    return iterator{&root_};
}

template <typename T, intrusive_list_node T::*member_node>
auto intrusive_list<T, member_node>::begin() const -> const_iterator
{
    return const_iterator{root_.next()};
}

template <typename T, intrusive_list_node T::*member_node>
auto intrusive_list<T, member_node>::end() const -> const_iterator
{
    return const_iterator{&root_};
}

template <typename T, intrusive_list_node T::*member_node>
bool intrusive_list<T, member_node>::empty() const
{
    return begin() == end();
}

template <typename T, intrusive_list_node T::*member_node>
T& intrusive_list<T, member_node>::front()
{
    assert(!empty());
    return *begin();
}

template <typename T, intrusive_list_node T::*member_node>
const T& intrusive_list<T, member_node>::front() const
{
    assert(!empty());
    return *begin();
}

template <typename T, intrusive_list_node T::*member_node>
T& intrusive_list<T, member_node>::back()
{
    assert(!empty());
    return *(--end());
}

template <typename T, intrusive_list_node T::*member_node>
const T& intrusive_list<T, member_node>::back() const
{
    assert(!empty());
    return *(--end());
}

template <typename T, intrusive_list_node T::*member_node>
auto intrusive_list<T, member_node>::insert(const_iterator pos, T& value) -> iterator
{
    auto node = detail::to_intrusive_list_node<T, member_node>(&value);
    node->link(const_cast<intrusive_list_node*>(pos.node()));
    return iterator{node};
}

template <typename T, intrusive_list_node T::*member_node>
auto intrusive_list<T, member_node>::push_back(T& value) -> iterator
{
    return insert(end(), value);
}

template <typename T, intrusive_list_node T::*member_node>
auto intrusive_list<T, member_node>::push_front(T& value) -> iterator
{
    return insert(begin(), value);
}

template <typename T, intrusive_list_node T::*member_node>
auto intrusive_list<T, member_node>::erase(const_iterator pos) -> iterator
{
    auto node = const_cast<intrusive_list_node*>(pos.node());
    if(node != &root_) {
        iterator next{node->next()};
        node->unlink();
        return next;
    }

    return end();
}

template <typename T, intrusive_list_node T::*member_node>
void intrusive_list<T, member_node>::pop_front()
{
    assert(!empty());
    erase(begin());
}

template <typename T, intrusive_list_node T::*member_node>
void intrusive_list<T, member_node>::pop_back()
{
    assert(!empty());
    erase(--end());
}

template <typename T, intrusive_list_node T::*member_node>
void intrusive_list<T, member_node>::clear()
{
    for(auto it = begin(); it != end(); ) {
        it = erase(it);
    }
}
