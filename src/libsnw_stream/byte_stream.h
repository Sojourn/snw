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
    basic_byte_stream(size_t min_size);
    basic_byte_stream(basic_byte_stream&&) = delete;
    basic_byte_stream(const basic_byte_stream&) = delete;

    basic_byte_stream& operator=(basic_byte_stream&&) = delete;
    basic_byte_stream& operator=(const basic_byte_stream&) = delete;

public:
    size_t writable() const;

    void write_begin();
    void write_commit();
    void write_rollback();

    template<size_t len>
    void* write();
    void* write(size_t len);

public:
    size_t readable() const;

    void read_begin();
    void read_commit();
    void read_rollback();

    template<size_t len>
    void* read();
    void* read(size_t len);

private:
    void* deref(size_t seq);

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

#include "byte_stream.hpp"
