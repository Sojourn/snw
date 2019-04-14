#pragma once

#include <atomic>
#include <cstring>
#include <cstddef>
#include <cstdint>
#include "stream_buffer.h"

namespace snw {

template<typename Sequence>
class basic_byte_stream {
public:
    basic_byte_stream(size_t min_size)
        : buffer_(min_size)
        , mask_(buffer_.size() - 1)
        , wwseq_(0)
        , wrseq_(0)
        , wseq_(0)
        , rwseq_(0)
        , rrseq_(0)
        , rseq_(0)
    {
        memset(pad0_, 0, sizeof(pad0_));
        memset(pad1_, 0, sizeof(pad1_));
        memset(pad2_, 0, sizeof(pad2_));
        memset(pad3_, 0, sizeof(pad3_));
    }

    void write_begin() {
        wrseq_ = rseq_;
    }

    size_t writable() const {
        return buffer_.size() - (wwseq_ - wrseq_);
    }

    template<size_t len>
    void* write() {
        if (writable() < len) {
            return nullptr;
        }

        void* buf = deref(wwseq_);
        wwseq_ += len;
        return buf;
    }

    void* write(size_t len) {
        if (writable() < len) {
            return nullptr;
        }

        void* buf = deref(wwseq_);
        wwseq_ += len;
        return buf;
    }

    void write_commit() {
        wseq_ = wwseq_;
    }

    void write_rollback() {
        wwseq_ = wseq_;
    }

    void read_begin() {
        rwseq_ = wseq_;
    }

    size_t readable() const {
        return rwseq_ - rrseq_;
    }

    template<size_t len>
    void* read() {
        if (readable() < len) {
            return nullptr;
        }

        void* buf = deref(rrseq_);
        rrseq_ += len;
        return buf;
    }

    void* read(size_t len) {
        if (readable() < len) {
            return nullptr;
        }

        void* buf = deref(rrseq_);
        rrseq_ += len;
        return buf;
    }

    void read_commit() {
        rseq_ = rrseq_;
    }

    void read_rollback() {
        rrseq_ = rseq_;
    }

private:
    void* deref(size_t seq) {
        return &buffer_.data()[seq & mask_];
    }

private:
    stream_buffer buffer_;
    size_t        mask_;

    uint8_t       pad0_[64];
    size_t        wwseq_; // writer's cached wseq
    size_t        wrseq_; // writer's cached rseq

    uint8_t       pad1_[64];
    Sequence      wseq_;

    uint8_t       pad2_[64];
    size_t        rwseq_; // reader's cached wseq
    size_t        rrseq_; // reader's cached rseq

    uint8_t       pad3_[64];
    Sequence      rseq_;
};

using byte_stream = basic_byte_stream<size_t>;
using atomic_byte_stream = basic_byte_stream<std::atomic_size_t>;

}
