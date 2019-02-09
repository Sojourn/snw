#pragma once

#include "object.h"
#include "object_heap.h"
#include "lexer.h"
#include "array.h"

namespace snw {

class parser {
    friend class lexer;
public:
    parser(object_heap* heap)
        : heap_(heap)
        , result_(make_nil_object(heap))
    {
    }

    object_handle parse(const char* str) {
        try {
            snw::parse(str, *this);
            object_handle result = result_;
            result_ = make_nil_object(*heap_);
            return result;
        }
        catch (const std::exception&) {
            result_ = make_nil_object(*heap_);
            stack_.clear();
            throw;
        }
    }

private:
    void open_file() {
        list_builder builder(*heap_);
        result_ = builder.back();
        stack_.push_back(builder);
    }

    void close_file() {
        if (stack_.size() != 1) {
            throw std::runtime_error("Missing ')'");
        }
        stack_.pop_back();
    }

    void open_list() {
        list_builder builder(*heap_);
        stack_.back().push_back(builder.back());
        stack_.push_back(builder);
    }

    void close_list() {
        if (stack_.size() == 1) {
            throw std::runtime_error("Missing '('");
        }
        stack_.pop_back();
    }

    void integer(int64_t value) {
        stack_.back().push_back(make_integer_object(*heap_, value));
    }

    void string(const char* first, const char* last) {
        stack_.back().push_back(make_string_object(*heap_, first, last););
    }

    void symbol(const char* first, const char* last) {
        stack_.back().push_back(make_symbol_object(*heap_, first, last));
    }

    void comment(const char* first, const char* last) {
        (void)first;
        (void)last;
    }

    void error(const snw::lexer_error& err) {
        throw std::runtime_error("lexer error");
    }

private:
    class list_builder {
    public:
        list_builder(object_heap& heap)
            : heap_(&heap)
            , tail_(make_cell_object(heap))
            , size_(0)
        {
        }

        cell_object back() const {
            return tail_;
        }

        void push_back(object_handle handle) {
            if (size_ == 0) {
                tail_.set_car(handle);
            }
            else {
                cell_object new_cell = make_cell_object(*heap_);
                new_cell.set_car(handle);
                tail_.set_cdl(new_cell);
                tail_ = new_cell;
            }

            ++size_;
        }

    private:
        object_heap*              heap_;
        object<object_type::cell> tail_;
        size_t                    size_;
    };

    object_heap*             heap_;
    object_handle            result_;
    array<list_builder, 512> stack_;
};

}
