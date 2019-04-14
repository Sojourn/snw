#pragma once

#include <cstring>
#include "byte_stream.h"

template<typename Sequence>
snw::basic_byte_stream<Sequence>::basic_byte_stream(size_t min_size)
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

template<typename Sequence>
size_t snw::basic_byte_stream<Sequence>::writable() const {
    return buffer_.size() - (wwseq_ - wrseq_);
}

template<typename Sequence>
void snw::basic_byte_stream<Sequence>::write_begin() {
    wrseq_ = rseq_;
}

template<typename Sequence>
void snw::basic_byte_stream<Sequence>::write_commit() {
    wseq_ = wwseq_;
}

template<typename Sequence>
void snw::basic_byte_stream<Sequence>::write_rollback() {
    wwseq_ = wseq_;
}

template<typename Sequence>
template<size_t len>
void* snw::basic_byte_stream<Sequence>::write() {
    if (writable() < len) {
        return nullptr;
    }

    void* buf = deref(wwseq_);
    wwseq_ += len;
    return buf;
}

template<typename Sequence>
void* snw::basic_byte_stream<Sequence>::write(size_t len) {
    if (writable() < len) {
        return nullptr;
    }

    void* buf = deref(wwseq_);
    wwseq_ += len;
    return buf;
}

template<typename Sequence>
size_t snw::basic_byte_stream<Sequence>::readable() const {
    return rwseq_ - rrseq_;
}

template<typename Sequence>
void snw::basic_byte_stream<Sequence>::read_begin() {
    rwseq_ = wseq_;
}

template<typename Sequence>
void snw::basic_byte_stream<Sequence>::read_commit() {
    rseq_ = rrseq_;
}

template<typename Sequence>
void snw::basic_byte_stream<Sequence>::read_rollback() {
    rrseq_ = rseq_;
}

template<typename Sequence>
template<size_t len>
void* snw::basic_byte_stream<Sequence>::read() {
    if (readable() < len) {
        return nullptr;
    }

    void* buf = deref(rrseq_);
    rrseq_ += len;
    return buf;
}

template<typename Sequence>
void* snw::basic_byte_stream<Sequence>::read(size_t len) {
    if (readable() < len) {
        return nullptr;
    }

    void* buf = deref(rrseq_);
    rrseq_ += len;
    return buf;
}

template<typename Sequence>
void* snw::basic_byte_stream<Sequence>::deref(size_t seq) {
    return &buffer_.data()[seq & mask_];
}
