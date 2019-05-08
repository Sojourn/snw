#include <iostream>
#include <chrono>
#include <vector>
#include <limits>
#include <string>
#include <atomic>
#include <memory>
#include <thread>
#include <algorithm>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include <tuple>
#include <cstdio>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cassert>

#include <unistd.h>

#include "snw_stream.h"
#include "align.h"
#include "varchar.h"
#include "intrusive_list.h"
#include "function.h"
#include "type_traits.h"
#include "find_type.h"
#include "bits.h"
#include "array.h"

namespace snw {

struct bit_tree_node {
    static const constexpr size_t capacity = 64 * 8; // 512
    static const constexpr size_t log2_capacity = 9; // log2(512) == 9
    static_assert(capacity == (1 << log2_capacity), "capacity and log2_capacity are out of sync");

    uint64_t bits[8];

    bit_tree_node() {
        clear();
    }

    void clear() {
        memset(bits, 0, sizeof(bits));
    }

    void clear_bit(size_t bit_index) {
        bits[bit_index >> 6] &= ~(1ull << (bit_index & 63));
    }

    void set_bit(size_t bit_index) {
        bits[bit_index >> 6] |= (1ull << (bit_index & 63));
    }
};

struct bit_tree_level {
    size_t node_count;
    size_t shift;
    size_t mask;
};

struct bit_tree_layout {
    static const constexpr size_t max_height = 4;
    static const constexpr size_t max_capacity = 1ull << (max_height * bit_tree_node::log2_capacity); // 64 << 30

    size_t                            capacity;
    size_t                            total_nodes;
    array<bit_tree_level, max_height> levels;

    bit_tree_layout(size_t min_capacity)
        : capacity(min_capacity)
        , total_nodes(0)
    {
        // sanity check that we aren't trying to create a crazy huge layout
        if (min_capacity > max_capacity) {
            throw std::runtime_error("cannot create a bit_tree_layout that large");
        }

        // capacity greater than zero
        if (capacity == 0) {
            capacity = 1;
        }

        // capacity should be a multiple of bit_tree_node::capacity
        capacity = align_up(capacity, bit_tree_node::capacity);

        // build the tree structure level-by-level
        {
            bit_tree_level level;
            auto add_level = [&]() {
                total_nodes += level.node_count;
                levels.push_back(level);
            };

            // add the bottom level first
            level.node_count = capacity / bit_tree_node::capacity;
            level.shift = 0;
            level.mask = (bit_tree_node::capacity - 1) << level.shift;
            add_level();

            // add levels until we get one with a single, root node
            while (level.node_count > 1) {
                level.node_count = align_up(level.node_count, bit_tree_node::capacity) / bit_tree_node::capacity;
                level.shift += bit_tree_node::log2_capacity;
                level.mask = level.mask << bit_tree_node::log2_capacity;
                add_level();
            }

            // levels should be in descending order
            std::reverse(levels.begin(), levels.end());
        }

        // check post-conditions 
        assert(capacity > 0);
        assert(is_aligned(capacity, bit_tree_node::capacity));
        assert(levels.front().node_count == 1);
        for (const bit_tree_level &level: levels) {
            assert(((level.node_count * bit_tree_node::capacity) << level.shift) >= capacity);
        }
    }

    friend std::ostream& operator<<(std::ostream& out, const bit_tree_layout& layout) {
        out << "bit_tree_layout {\n";
        out << "  capacity: " << layout.capacity << ",\n";
        out << "  total_nodes: " << layout.total_nodes << ",\n";
        out << "  approximate_memory_usage: " << (layout.total_nodes * sizeof(bit_tree_node)) << ",\n";
        out << "  approximate_bits_per_value: " << (float(8 * layout.total_nodes * sizeof(bit_tree_node)) / float(layout.capacity)) << ",\n";
        out << "  levels: [\n";
        for (const bit_tree_level& level: layout.levels) {
            out << "    { node_count: " << level.node_count << ", shift: " << level.shift << ", mask: " << level.mask << " },\n";
        }
        out << "  ]\n";
        out << '}';
    }
};

class bit_tree {
public:
    bit_tree(size_t min_capacity)
        : layout_(min_capacity)
    {
        nodes_.resize(layout_.total_nodes);
        
        size_t offset = 0;
        for (const bit_tree_level& level: layout_.levels) {
            levels_.push_back(&nodes_[offset]);
            offset += level.node_count;
        }
    }

private:
    bit_tree_layout                                    layout_;
    array<bit_tree_node*, bit_tree_layout::max_height> levels_;
    std::vector<bit_tree_node>                         nodes_;
};

}

int main(int argc, const char** argv) {
    // std::cout << snw::bit_tree_layout::max_height << std::endl;

    std::cout << snw::bit_tree_layout(13) << std::endl;
    std::cout << snw::bit_tree_layout(512) << std::endl;
    std::cout << snw::bit_tree_layout(513) << std::endl;
    std::cout << snw::bit_tree_layout(128 << 20) << std::endl;

    return 0;
}
