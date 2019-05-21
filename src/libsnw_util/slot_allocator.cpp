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

snw::slot_allocator::slot_allocator(size_t capacity)
    : capacity_(capacity)
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
    if (size() == capacity()) {
        return false;
    }

    cursor c;

    for (size_t depth = 0; depth < height_; ++depth) {
        node& node = levels_[depth].first[c.tell()];

        int chunk_offset = 0;
        int bit_offset = 0;

        if (is_leaf_level(depth)) {
            leaf_node& leaf = static_cast<leaf_node&>(node);
            for (; chunk_offset < node_chunk_count; ++chunk_offset) {
                uint64_t& live_chunk = leaf.live[chunk_offset];
                uint64_t dead_chunk = ~live_chunk;
                if (dead_chunk) {
                    bit_offset = count_trailing_zeros(dead_chunk);
                    break;
                }
            }
        }
        else {
            branch_node& branch = static_cast<branch_node&>(node);
            for (; chunk_offset < node_chunk_count; ++chunk_offset) {
                uint64_t& dead_chunk = branch.any_dead[chunk_offset];
                if (dead_chunk) {
                    bit_offset = count_trailing_zeros(dead_chunk);
                    break;
                }
            }
        }

        assert(chunk_offset < node_chunk_count);
        c.desend(chunk_offset, bit_offset);
    }

    *slot = c.tell();

    bool any_live = false;
    bool any_dead = false;

    for (size_t i = 0; i < height_; ++i) {
        int chunk_offset = c.chunk_offset();
        int bit_offset = c.bit_offset();
        c.ascend();

        size_t depth = height_ - i - 1;
        size_t node_index = c.tell();
        node& node = levels_[depth].first[node_index];

        if (is_leaf_level(depth)) {
            leaf_node& leaf = static_cast<leaf_node&>(node);

            assert(!test_bit(leaf.live[chunk_offset], bit_offset));
            set_bit(leaf.live[chunk_offset], bit_offset);
            any_live = true;
            any_dead = !all(leaf.live);
        }
        else {
            branch_node& branch = static_cast<branch_node&>(node);

            if (any_live) {
                set_bit(branch.any_live[chunk_offset], bit_offset);
                any_live = true;
            }

            if (!any_dead) {
                clear_bit(branch.any_dead[chunk_offset], bit_offset);
                any_dead = none(branch.any_dead);
            }
        }
    }

    return true;
}

void snw::slot_allocator::deallocate(uint64_t slot) {
    cursor c(slot);

    bool any_live = false;
    bool any_dead = false;

    // TODO: detect if we can stop iteration early
    for (size_t i = 0; i < height_; ++i) {
        int chunk_offset = c.chunk_offset();
        int bit_offset = c.bit_offset();
        c.ascend();

        level& level = levels_[height_ - i - 1];
        node& node = level.first[c.tell()];

        if (i == 0) {
            leaf_node& leaf = static_cast<leaf_node&>(node);

            assert(test_bit(leaf.live[chunk_offset], bit_offset));
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

bool snw::slot_allocator::is_leaf_level(size_t depth) const {
    return (depth == (height_ - 1));
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
