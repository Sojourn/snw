#pragma once

#include "intrusive_list.h"
#include "array.h"
#include "page.h"

namespace snw {

class page_list {
public:
    page_list();
    ~page_list();

    bool empty() const;

    void push_front(page* page);
    void push_back(page* page);
    page* pop_front();
    page* pop_back();

    // iterator begin();
    // const_iterator begin() const;
    // iterator end();
    // const_iterator end() const;

private:
    struct page_node {
        intrusive_list_node node;
        array<page*, (4096 - sizeof(intrusive_list_node) - sizeof(size_t)) / sizeof(page*)> pages;
    };
    static_assert(sizeof(page_node) == 4096, "");

    using page_node_list = intrusive_list<page_node, &page_node::node>;

    page_node_list page_nodes_;
};

}
