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

template<typename F>
inline void snw::slot_allocator::scan(F&& f) const {
    // TODO
}
