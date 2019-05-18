#include <algorithm>
#include <cassert>
#include "align.h"
#include "bits.h"
#include "slot_allocator.h"

class snw::slot_allocator::cursor {
public:
    cursor(uint64_t index = 0) {
        seek(index);
    }

    void seek(uint64_t index) {
        index_ = index;
    }

    uint64_t tell() const {
        return index_;
    }

    void desend(int chunk_offset, int bit_offset) {
        index_ = (index_ << 8) | (chunk_offset << 6) | (bit_offset << 0);
    }

    void ascend() {
        index_ = index_ >> 8;
    }

    int chunk_offset() const {
        return 3 & (index_ >> 6);
    }

    int bit_offset() const {
        return 63 & (index_ >> 0);
    }

private:
    uint64_t index_;
};

snw::slot_allocator::slot_allocator(size_t min_capacity)
    : capacity_(0)
    , size_(0)
    , height_(0)
{
    size_t branch_count = 0;
    size_t leaf_count = 0;

    // compute the tree layout
    {
        size_t width = align_up(capacity_, node_capacity) / node_capacity;
        levels_[height_].width = width;
        leaf_count += width;
        ++height_;

        while (width > 1) {
            width = align_up(width, node_capacity) / node_capacity;
            levels_[height_].width = width;
            branch_count += width;
            ++height_;
        }

        // put levels in desending order
        std::reverse(levels_, levels_ + height_);
    }

    // setup the tree
    {
        size_t depth = 0;

        if (branch_count > 0) {
            branch_nodes_.resize(branch_count);

            size_t branch_offset = 0;
            for (; depth < (height_ - 1); ++depth) {
                levels_[depth].first = &branch_nodes_[branch_offset];
                branch_offset += levels_[depth].width;
            }
        }

        leaf_nodes_.resize(leaf_count);
        levels_[depth].first = leaf_nodes_.data();
    }
}

bool snw::slot_allocator::allocate(uint64_t* slot) {
    return -1;
}

void snw::slot_allocator::deallocate(uint64_t slot) {
    cursor c(slot);

    bool any_live = false;
    bool any_dead = false;

    // TODO: detect if we can stop iteration early
    for (size_t i = 0; (i < height_); ++i) {
        int chunk_offset = c.chunk_offset();
        int bit_offset = c.bit_offset();
        c.ascend();

        level& level = levels_[height_ - i - 1];
        node& node = level.first[c.tell()];

        if (i == 0) {
            leaf_node& leaf = static_cast<leaf_node&>(node);

            clear_bit(leaf.live[chunk_offset], bit_offset);
            any_live = any(leaf.live);
            any_dead = true;
        }
        else {
            branch_node& branch = static_cast<branch_node&>(node);

            if (!any_live) {
                clear_bit(branch.any_live[chunk_offset], bit_offset);
                any_live = any(branch.any_live);
            }

            if (any_dead) {
                set_bit(branch.any_dead[chunk_offset], bit_offset);
                any_dead = true;
            }
        }
    }
}

bool snw::slot_allocator::any(const uint64_t (&chunks)[node_chunk_count]) {
    for (uint64_t chunk: chunks) {
        if (chunk != 0) {
            return true;
        }
    }

    return false;
}

bool snw::slot_allocator::all(const uint64_t (&chunks)[node_chunk_count]) {
    for (uint64_t chunk: chunks) {
        if (chunk != std::numeric_limits<uint64_t>::max()) {
            return false;
        }
    }

    return true;
}

bool snw::slot_allocator::none(const uint64_t (&chunks)[node_chunk_count]) {
    return !any(chunks);
}
