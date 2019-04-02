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

class message_stream {
public:
    message_stream(size_t size)
        : buffer_(size)
        , mask_(buffer_.size() - 1)
        , wseq_(0)
        , rseq_(0)
    {
    }

    bool write(const char* msg) {
        size_t len = strlen(msg) + 1;
        if (len > writable()) {
            return false;
        }

        memcpy(buffer_.data() + (wseq_ & mask_), msg, len);
        wseq_ += len;
        return true;
    }

    const char* read() {
        if (readable() == 0) {
            return nullptr;
        }

        const char* msg = reinterpret_cast<const char*>(buffer_.data() + (rseq_ & mask_));
        rseq_ += (strlen(msg) + 1);
        return msg;
    }

    size_t readable() const {
        return (wseq_ - rseq_);
    }

    size_t writable() const {
        return buffer_.size() - readable();
    }

private:
    stream_buffer buffer_;
    size_t        mask_;
    size_t        wseq_;
    size_t        rseq_;
};

class spsc_queue {
public:
    spsc_queue(size_t min_size)
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

    void* write(size_t len) {
        if ((buffer_.size() - (wwseq_ - wrseq_)) < len) {
            return nullptr;
        }

        void* buf = &buffer_.data()[wwseq_ & mask_];
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

    const void* read(size_t len) {
        if ((rwseq_ - rrseq_) < len) {
            return nullptr;
        }

        const void* buf = &buffer_.data()[rrseq_ & mask_];
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
    stream_buffer      buffer_;
    size_t             mask_;

    uint8_t            pad0_[64];
    size_t             wwseq_; // writer's cached wseq
    size_t             wrseq_; // writer's cached rseq

    uint8_t            pad1_[64];
    std::atomic_size_t wseq_;

    uint8_t            pad2_[64];
    size_t             rwseq_; // reader's cached wseq
    size_t             rrseq_; // reader's cached rseq

    uint8_t            pad3_[64];
    std::atomic_size_t rseq_;
};

class spsc_queue_writer {
public:
    spsc_queue_writer(spsc_queue& queue)
        : queue_(queue)
        , done_(false)
    {
        queue_.write_begin();
    }

    spsc_queue_writer(spsc_queue_writer&&) = delete;
    spsc_queue_writer(const spsc_queue_writer&) = delete;

    ~spsc_queue_writer() {
        if (!done_) {
            queue_.write_rollback();
        }
    }

    spsc_queue_writer& operator=(spsc_queue_writer&&) = delete;
    spsc_queue_writer& operator=(const spsc_queue_writer&) = delete;

    template<typename T>
    void write(const T& val) {
        assert(!done_);

        void* buf = queue_.write(sizeof(val));
        if (!buf) {
            throw std::runtime_error("write failed");
        }

        memcpy(buf, &val, sizeof(val));
    }

    void commit() {
        assert(!done_);
        queue_.write_commit();
        done_ = true;
    }

private:
    spsc_queue& queue_;
    bool        done_;
};

class spsc_queue_reader {
public:
    spsc_queue_reader(spsc_queue& queue)
        : queue_(queue)
        , done_(false)
    {
        queue_.read_begin();
    }

    spsc_queue_reader(spsc_queue_reader&&) = delete;
    spsc_queue_reader(const spsc_queue_reader&) = delete;

    ~spsc_queue_reader() {
        if (!done_) {
            queue_.read_rollback();
        }
    }

    spsc_queue_reader& operator=(spsc_queue_reader&&) = delete;
    spsc_queue_reader& operator=(const spsc_queue_reader&) = delete;

    template<typename T>
    bool try_read(T& val) {
        const void* buf = queue_.read(sizeof(val));
        if (!buf) {
            return false;
        }

        memcpy(&val, buf, sizeof(val));
        return true;
    }

    template<typename T>
    T read() {
        T val;
        if (!try_read(val)) {
            throw std::runtime_error("read failed");
        }

        return val;
    }

    void commit() {
        assert(!done_);
        queue_.read_commit();
        done_ = true;
    }

private:
    spsc_queue& queue_;
    bool        done_;
    const void* buf_;
    size_t      len_;
    size_t      off_;
};

}

int main(int argc, const char** argv) {
    try {
        snw::spsc_queue queue(1 << 16);
        std::vector<char> str;

        for (int j = 0; j < (1 << 20); ++j) {
            {
                snw::spsc_queue_writer writer(queue);
                for (char c: "hello, world") {
                    writer.write(c);
                }
                writer.commit();
            }
            {
                snw::spsc_queue_reader reader(queue);

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
