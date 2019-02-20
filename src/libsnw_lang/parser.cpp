#include <stdexcept>
#include <iostream>
#include "object_transaction.h"
#include "parser.h"

using namespace snw;

parser::parser(object_heap& heap)
    : heap_(heap)
{
}

object_ref parser::parse(const char* str) {
    try {
        object_transaction txn(&heap_);
        snw::parse(str, *this);
        txn.commit();
        return program_;
    }
    catch (const std::exception&) {
        stack_.clear();
        throw;
    }
}

void parser::open_file() {
    stack_.push_back(frame{});
}

void parser::close_file() {
    if (stack_.size() != 1) {
        throw std::runtime_error("unclosed list");
    }

    program_ = pop_list();
}

void parser::open_list() {
    stack_.push_back(frame{});
}

void parser::close_list() {
    if (stack_.size() == 1) {
        throw std::runtime_error("unopened list");
    }

    object_ref ref = pop_list();
    stack_.back().push_back(ref);
}

void parser::integer(int64_t value) {
    stack_.back().push_back(heap_.new_integer(value));
}

void parser::string(const char* first, const char* last) {
    stack_.back().push_back(heap_.new_string(first, last));
}

void parser::symbol(const char* first, const char* last) {
    stack_.back().push_back(heap_.new_symbol(first, last));
}

void parser::comment(const char* first, const char* last) {
    (void)first;
    (void)last;
}

void parser::error(const snw::lexer_error& err) {
    throw std::runtime_error(err.msg);
}

object_ref parser::pop_list() {
    auto& frame = stack_.back();
    auto first = frame.data();
    auto last = first + frame.size();

    object_ref ref = heap_.new_list(first, last);
    stack_.pop_back();
    return ref;
}
