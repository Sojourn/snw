#pragma once

#include <cassert>
#include "object_heap.h"
#include "object_stack.h"

namespace snw {

class object_transaction {
public:
    object_transaction(object_heap& heap, object_stack& stack)
        : heap_(heap)
        , stack_(stack)
        , finished_(false)
    {
        heap_.begin();
        stack_.begin();
    }

    object_transaction(const object_transaction&) = delete;
    object_transaction(object_transaction&&) = delete;

    ~object_transaction() {
        if (!finished_) {
            heap_.rollback();
            stack_.rollback();
        }
    }

    object_transaction& operator=(object_transaction&&) = delete;
    object_transaction& operator=(const object_transaction&) = delete;

    void commit() {
        assert(!finished_);
        heap_.commit();
        stack_.commit();
        finished_ = true;
    }

    void rollback() {
        assert(!finished_);
        heap_.rollback();
        stack_.rollback();
        finished_ = true;
    }

private:
    object_heap&  heap_;
    object_stack& stack_;
    bool          finished_;
};

}
