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

// traversing set or clear bits?
enum class bit_tree_traversal_mode {
    set,
    clear,
};

struct bit_tree_node {
    static const constexpr size_t chunk_count = 4;
    static const constexpr size_t capacity = 64 * chunk_count; // 512
    static const constexpr size_t log2_capacity = 8; // log2(256) == 8
    static_assert(capacity == (1 << log2_capacity), "capacity and log2_capacity are out of sync");
};

// these have aggregate information about child nodes
struct alignas(64) bit_tree_branch_node : bit_tree_node {
    uint64_t any_set_chunks[chunk_count];
    uint64_t any_clear_chunks[chunk_count];

    bit_tree_branch_node() {
        for (size_t i = 0; i < chunk_count; ++i) {
            any_set_chunks[i] = std::numeric_limits<uint64_t>::min();
            any_clear_chunks[i] = std::numeric_limits<uint64_t>::max();
        }
    }

    uint64_t* get_chunks(bit_tree_traversal_mode mode) {
        if (mode == bit_tree_traversal_mode::set) {
            return any_set_chunks;
        }
        else {
            return any_clear_chunks;
        }
    }

    const uint64_t* get_chunks(bit_tree_traversal_mode mode) const {
        if (mode == bit_tree_traversal_mode::set) {
            return any_set_chunks;
        }
        else {
            return any_clear_chunks;
        }
    }
};

struct alignas(16) bit_tree_leaf_node : bit_tree_node {
    uint64_t chunks[chunk_count];

    bit_tree_leaf_node() {
        for (size_t i = 0; i < chunk_count; ++i) {
            chunks[i] = std::numeric_limits<uint64_t>::min();
        }
    }

    uint64_t* get_chunks() {
        return chunks;
    }

    const uint64_t* get_chunks() const {
        return chunks;
    }
};

struct bit_tree_level {
    size_t node_count;
    size_t shift;
    size_t mask;
};

struct bit_tree_layout {
    static const constexpr size_t max_height = 4;
    static const constexpr size_t max_internal_height = max_height - 1;
    static const constexpr size_t max_capacity = 1ull << 31;

    // sanity check the constants
    static_assert((1ull << (max_height * bit_tree_node::log2_capacity)) >= max_capacity, "increase height");
    static_assert(std::numeric_limits<int32_t>::max() >= (max_capacity - 1), "index type is too small for max_capacity");

    size_t                            capacity;
    size_t                            node_count;
    size_t                            leaf_node_count;
    size_t                            branch_node_count;
    array<bit_tree_level, max_height> levels;

    bit_tree_layout(size_t min_capacity)
        : capacity(min_capacity)
        , node_count(0)
        , leaf_node_count(0)
        , branch_node_count(0)
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
                node_count += level.node_count;
                if (levels.empty()) {
                    leaf_node_count += level.node_count;
                }
                else {
                    branch_node_count += level.node_count;
                }

                levels.push_back(level);
            };

            // add a level of leaves
            level.node_count = capacity / bit_tree_node::capacity;
            level.shift = 0;
            level.mask = (bit_tree_node::capacity - 1) << level.shift;
            add_level();

            // add levels of branches until we find the root node
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
        out << "  node_count: " << layout.node_count << ",\n";
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
        : size_(0)
        , layout_(min_capacity)
    {
        leaves_.resize(layout_.leaf_node_count);
        branches_.resize(layout_.branch_node_count);

        // setup branch index
        size_t offset = 0;
        for (size_t i = 0; i < (layout_.levels.size() - 1); ++i) {
            branch_index_.push_back(&branches_[offset]);
            offset += layout_.levels[i].node_count;
        }
    }

    int32_t add() {
        if (full()) {
            return -1;
        }

        bit_tree_cursor<bit_tree_traversal_mode::clear> bit_tree_cursor(*this);
        bit_tree_index index = bit_tree_cursor.tell();
        int32_t result = static_cast<int32_t>(index.get());

        // todo: set bit
        size_t height = layout_.levels.size();
        for (size_t i = 0; i < height; ++i) {
            int chunk_index = index.top_chunk();
            int bit_index = index.top_bit();
        }

        return result;
    }

    void remove(int32_t index) {
    }

    template<typename F>
    void scan(F&& f) /* const */ {
        // todo
    }

    size_t size() const {
        return size_;
    }

    size_t capacity() const {
        return layout_.capacity;
    }

    bool full() const {
        return size() == capacity();
    }

    bool empty() const {
        return size() == 0;
    }

private:
    class bit_tree_index {
    public:
        bit_tree_index(uint32_t index = 0)
            : index_(index)
        {
        }

        void push_node(int chunk, int bit) {
            index_ = (index_ << 8) | (chunk << 6) | (bit << 0);
        }

        void pop_node() {
            index_ = index_ >> 8;
        }

        int top_chunk() const {
            return 3 & (index_ >> 6);
        }

        int top_bit() const {
            return 63 & (index_ >> 0);
        }

        uint32_t get() const {
            return index_;
        }

    private:
        uint32_t index_;
    };

    template<bit_tree_traversal_mode mode>
    class bit_tree_cursor {
    public:
        bit_tree_cursor(bit_tree& bt)
            : bt_(bt)
        {
            if (mode == bit_tree_traversal_mode::set) {
                assert(!bt_.empty());
            }
            else {
                assert(!bt_.full());
            }

            {
                // traverse the branch level
                size_t height = bt_.layout_.levels.size();
                for (size_t current_depth = 0; current_depth < (height - 1); ++current_depth) {
                    const bit_tree_branch_node& branch = bt_.branch_index_[current_depth][index_.get()];
                    const uint64_t* chunks = branch.get_chunks(mode);

                    // scan chunks
                    for (size_t chunk_index = 0; chunk_index < bit_tree_node::chunk_count; ++chunk_index) {
                        uint64_t chunk = chunks[chunk_index];
                        if (chunk) {
                            int bit_index = count_trailing_zeros(chunk);
                            index_.push_node(static_cast<int>(chunk_index), bit_index);
                            break;
                        }
                    }
                }
            }

            // traverse the leaf level
            {
                const bit_tree_leaf_node& leaf = bt_.leaves_[index_.get()];
                const uint64_t* chunks = leaf.get_chunks();

                // scan chunks
                for (size_t chunk_index = 0; chunk_index < bit_tree_node::chunk_count; ++chunk_index) {
                    uint64_t chunk = chunks[chunk_index];
                    if (mode == bit_tree_traversal_mode::clear) {
                        chunk = ~chunk; // invert chunk because we're looking for clear bits
                    }
                    if (chunk) {
                        int bit_index = count_trailing_zeros(chunk);
                        index_.push_node(static_cast<int>(chunk_index), bit_index);
                        break;
                    }
                }
            }
        }

        bit_tree_cursor(bit_tree& bt, uint32_t index)
            : bt_(bt)
            , index_(index)
        {
            assert(index < bt.capacity());
            if (mode == bit_tree_traversal_mode::set) {
                assert(bt.size() > 0);
            }
            else {
                assert(bt.size() < bt.capacity());
            }
        }

        int32_t tell() const {
            return static_cast<int32_t>(index_.get());
        }

    private:
        struct frame {
            size_t node_offset;
            size_t chunk_offset;
            size_t bit_offset;
        };

    private:
        bit_tree      &bt_;
        bit_tree_index index_;
    };

private:
    using branch_index = array<bit_tree_branch_node*, bit_tree_layout::max_internal_height>;

    size_t                            size_;
    bit_tree_layout                   layout_;
    branch_index                      branch_index_;
    std::vector<bit_tree_leaf_node>   leaves_;
    std::vector<bit_tree_branch_node> branches_;
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
