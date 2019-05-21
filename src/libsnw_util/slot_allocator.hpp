#include "bits.h"

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
        index_ = (index_ >> 8);
    }

    uint64_t node_offset() const {
        return (index_ >> 8);
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

template<typename F>
inline void snw::slot_allocator::scan_node(int depth, uint64_t node_offset, F&& f) const {
    cursor cursor(node_offset);

    node& node = levels_[depth].first[node_offset];
    if (is_leaf_level(depth)) {
        leaf_node& leaf = static_cast<leaf_node&>(node);

        for (int chunk_offset = 0; chunk_offset < node_chunk_count; ++chunk_offset) {
            uint64_t chunk = leaf.live[chunk_offset];
            for_each_set_bit(chunk, [&](int bit_offset) {
                cursor.desend(chunk_offset, bit_offset);
                f(cursor.tell());
                cursor.ascend();
            });
        }
    }
    else {
        branch_node& branch = static_cast<branch_node&>(node);

        for (int chunk_offset = 0; chunk_offset < node_chunk_count; ++chunk_offset) {
            uint64_t chunk = branch.any_live[chunk_offset];
            for_each_set_bit(chunk, [&](int bit_offset) {
                cursor.desend(chunk_offset, bit_offset);
                scan_node(depth + 1, cursor.tell(), f);
                cursor.ascend();
            });
        }
    }
}

template<typename F>
inline void snw::slot_allocator::scan(F&& f) const {
    scan_node(0, 0, f);
}
