#pragma once

#include <stdexcept>
#include "object_heap.h"
#include "object_stack.h"

namespace snw {

class object_transaction {
public:
    object_transaction(object_heap* heap, object_stack* stack = nullptr)
        : heap_(heap)
        , stack_(stack)
        , finished_(false)
    {
        if (heap_) {
            heap_->begin();
        }
        if (stack_) {
            stack_->begin();
        }
    }

    object_transaction(const object_transaction&) = delete;
    object_transaction(object_transaction&&) = delete;

    ~object_transaction() {
        if (!finished_) {
            rollback();
        }
    }

    object_transaction& operator=(object_transaction&&) = delete;
    object_transaction& operator=(const object_transaction&) = delete;

    void commit() {
        if (finished_) {
            throw std::runtime_error("invalid transaction state");
        }

        if (stack_) {
            stack_->commit();
        }
        if (heap_) {
            heap_->commit();
        }

        finished_ = true;
    }

    void rollback() {
        if (finished_) {
            throw std::runtime_error("invalid transaction state");
        }

        if (stack_) {
            stack_->rollback();
        }
        if (heap_) {
            heap_->rollback();
        }

        finished_ = true;
    }

private:
    object_heap*  heap_;
    object_stack* stack_;
    bool          finished_;
};

}
