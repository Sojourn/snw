#include "page_list.h"

snw::page_list::page_list()
{
}

snw::page_list::~page_list()
{
}

bool snw::page_list::empty() const
{
    return page_nodes_.empty();
}

void snw::page_list::push_front(page* page)
{
    if(empty() || page_nodes_.front().pages.full()) {
        void* ptr = page;
        page->~page();

        auto node = new(ptr) page_node;
        page_nodes_.push_front(*node);
    }
    else {
        auto& pages = page_nodes_.front().pages;
        pages.insert(pages.begin(), page); // TODO: queue instead of array
    }
}

void snw::page_list::push_back(page* page)
{
    if(empty() || page_nodes_.back().pages.full()) {
        void* ptr = page;
        page->~page();

        auto node = new(ptr) page_node;
        page_nodes_.push_back(*node);
    }
    else {
        auto& pages = page_nodes_.back().pages;
        pages.insert(pages.begin(), page);
    }
}

snw::page* snw::page_list::pop_front()
{
    assert(!empty());

    page_node& node = page_nodes_.front();
    if(node.pages.empty()) {
        void* ptr = &node;
        node.~page_node();
        return new(ptr) page;
    }
    else {
        page* result = node.pages.front();
        node.pages.erase(node.pages.begin()); // TODO: use a queue instead of an array
        return result;
    }
}

snw::page* snw::page_list::pop_back()
{
    assert(!empty());

    page_node& node = page_nodes_.back();
    if(node.pages.empty()) {
        void* ptr = &node;
        node.~page_node();
        return new(ptr) page;
    }
    else {
        page* result = node.pages.back();
        node.pages.erase(node.pages.end());
        return result;
    }
}
