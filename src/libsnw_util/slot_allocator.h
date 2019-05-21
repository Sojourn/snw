#pragma once

#include <limits>
#include <vector>
#include <cstdint>
#include <cstddef>

#include "function.h"

namespace snw {

class slot_allocator {
public:
    slot_allocator(size_t capacity);

    // return the lowest available slot, or -
    bool allocate(uint64_t* slot);

    // free a slot
    void deallocate(uint64_t slot);

    size_t size() const {
        return size_;
    }

    size_t capacity() const {
        return capacity_;
    }

    bool full() const {
        return size_ == capacity_;
    }

    bool empty() const {
        return size_ < capacity_;
    }

private:
    static constexpr size_t max_height = 8;
    static constexpr size_t max_internal_height = max_height - 1;
    static constexpr size_t node_chunk_count = 4;
    static constexpr size_t node_capacity = node_chunk_count * sizeof(uint64_t) * 8;

    // TODO: implement these
    slot_allocator(slot_allocator&&) = delete;
    slot_allocator(const slot_allocator&) = delete;
    slot_allocator& operator=(slot_allocator&&) = delete;
    slot_allocator& operator=(const slot_allocator&) = delete;

    static bool any(const uint64_t (&chunks)[node_chunk_count]);
    static bool all(const uint64_t (&chunks)[node_chunk_count]);
    static bool none(const uint64_t (&chunks)[node_chunk_count]);

    bool is_leaf_level(size_t depth) const;

private:
    class cursor;

    struct node {};

    struct branch_node : node {
        uint64_t any_live[node_chunk_count]; // a mask of child nodes containing any allocated slots
        uint64_t any_dead[node_chunk_count]; // a mast of child nodes containing any deallocated slots
    };

    struct leaf_node : node {
        uint64_t live[node_chunk_count]; // a mask of allocated slots
    };

    struct level {
        size_t width;
        node*  first;
    };

    size_t                   capacity_;
    size_t                   size_;
    size_t                   height_;

    level                    levels_[max_height];
    std::vector<branch_node> branch_nodes_;
    std::vector<leaf_node>   leaf_nodes_;
};

}
