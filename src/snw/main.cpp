#include <iostream>
#include <vector>
#include <limits>
#include <string>
#include <atomic>
#include <stdexcept>
#include <cstdio>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cassert>

#include <unistd.h>

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
    const void* read() {
        if (readable() < len) {
            return nullptr;
        }

        const void* buf = deref(rrseq_);
        rrseq_ += len;
        return buf;
    }

    const void* read(size_t len) {
        if (readable() < len) {
            return nullptr;
        }

        const void* buf = deref(rrseq_);
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

    const void* deref(size_t seq) const {
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

template<typename ByteStream>
class byte_stream_writer {
public:
    byte_stream_writer(ByteStream& stream)
        : stream_(stream)
        , done_(false)
    {
        stream_.write_begin();
    }

    byte_stream_writer(byte_stream_writer&&) = delete;
    byte_stream_writer(const byte_stream_writer&) = delete;

    ~byte_stream_writer() {
        if (!done_) {
            stream_.write_rollback();
        }
    }

    byte_stream_writer& operator=(byte_stream_writer&&) = delete;
    byte_stream_writer& operator=(const byte_stream_writer&) = delete;

    template<typename T>
    void write(const T& val) {
        assert(!done_);

        void* buf = stream_.write<sizeof(val)>();
        if (!buf) {
            throw std::runtime_error("write failed");
        }

        memcpy(buf, &val, sizeof(val));
    }

    void write(const void* src_buf, size_t len) {
        assert(!done_);

        void* dst_buf = stream_.write(len);
        if (!dst_buf) {
            throw std::runtime_error("write failed");
        }

        memcpy(dst_buf, src_buf, len);
    }

    void commit() {
        assert(!done_);
        stream_.write_commit();
        done_ = true;
    }

private:
    ByteStream& stream_;
    bool        done_;
};

template<typename ByteStream>
class byte_stream_reader {
public:
    byte_stream_reader(ByteStream& stream)
        : stream_(stream)
        , done_(false)
    {
        stream_.read_begin();
    }

    byte_stream_reader(byte_stream_reader&&) = delete;
    byte_stream_reader(const byte_stream_reader&) = delete;

    ~byte_stream_reader() {
        if (!done_) {
            stream_.read_rollback();
        }
    }

    byte_stream_reader& operator=(byte_stream_reader&&) = delete;
    byte_stream_reader& operator=(const byte_stream_reader&) = delete;

    template<typename T>
    bool try_read(T& val) {
        const void* buf = stream_.read<sizeof(val)>();
        if (!buf) {
            return false;
        }

        memcpy(&val, buf, sizeof(val));
        return true;
    }

    template<typename T>
    T read() {
        T val;
        const void* buf = stream_.read(sizeof(val));
        if (!buf) {
            throw std::runtime_error("read failed");
        }

        memcpy(&val, buf, sizeof(val));
        return val;
    }

    void commit() {
        assert(!done_);
        stream_.read_commit();
        done_ = true;
    }

private:
    ByteStream& stream_;
    bool        done_;
};

}

int main(int argc, const char** argv) {
    try {
        snw::byte_stream stream(1 << 16);
        std::vector<char> str;

        for (int j = 0; j < (1 << 20); ++j) {
            {
                snw::byte_stream_writer<snw::byte_stream> writer(stream);
                for (char c: "hello, world") {
                    writer.write(c);
                }
                writer.commit();
            }
            {
                snw::byte_stream_reader<snw::byte_stream> reader(stream);

                char c;
                str.clear();
                while (reader.try_read(c)) {
                    str.push_back(c);
                }
                str.push_back('\n');
                reader.commit();

                write(1, str.data(), str.size());
            }
        }
    }
    catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
    }

    return 0;
}
