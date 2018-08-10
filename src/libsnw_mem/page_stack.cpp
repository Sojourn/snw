#include "page_stack.h"
#include <cstring>
#include <new>

snw::page_stack::page_stack()
    : head_(nullptr)
{
}

snw::page_stack::page_stack(page_stack&& other)
{
    head_ = other.head_;
    other.head_ = nullptr;
}

snw::page_stack::~page_stack()
{
    assert(empty());
}

snw::page_stack& snw::page_stack::operator=(page_stack&& rhs)
{
    if(this != &rhs) {
        assert(empty());

        head_ = rhs.head_;
        rhs.head_ = nullptr;
    }

    return *this;
}

void snw::page_stack::push_node(page* page)
{
    void* ptr = page;
    page->~page();

    node* head = new(ptr) node;
    memset(&head->header, 0, sizeof(head->header));

    head->header.prev = head_;
    head_ = head;
}

snw::page* snw::page_stack::pop_node()
{
    assert(head_);
    assert(head_->header.top == 0);

    auto head = head_;
    head_ = head->header.prev;

    void* ptr = head;
    head->~node();
    return new(ptr) page;
}
