#pragma once

#include <cassert>
#include "types.h"
#include "page.h"

namespace snw {

class page_stack {
public:
    page_stack();
    page_stack(page_stack&& other);
    page_stack(const page_stack&) = delete;
    ~page_stack();

    page_stack& operator=(page_stack&& rhs);
    page_stack& operator=(const page_stack&) = delete;

    inline bool empty() const {
        return !head_;
    }

    inline void push_back(page* page) {
        if(!head_ || (head_->header.top == node::capacity)) {
            push_node(page);
            return;
        }

        auto head = reinterpret_cast<node*>(head_);
        head->pages[head->header.top++] = page;
    }

    inline page* pop_back() {
        if(!head_) {
            return nullptr;
        }

        if(head_->header.top == 0) {
            return pop_node();
        }

        auto head = reinterpret_cast<node*>(head_);
        return head->pages[--head->header.top];
    }

private:
    void push_node(page* page);
    page* pop_node();

private:
    struct node;

    struct node_header {
        node*    prev;
        uint32_t top;
        uint32_t reserved;
    };

    struct node {
        static constexpr uint32_t capacity = (sizeof(page) - sizeof(node_header)) / sizeof(page*);

        node_header header;
        page*       pages[capacity];
    };
    static_assert(sizeof(node) == sizeof(page), "");

    node* head_;
};

}