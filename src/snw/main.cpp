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

    void batch_write_begin() {
        wrseq_ = rseq_;
    }

    bool batch_write_append(const void* buf, size_t len) {
        if ((buffer_.size() - (wwseq_ - wrseq_)) < len) {
            return false;
        }

        memcpy(&buffer_.data()[wwseq_ & mask_], buf, len);
        wwseq_ += len;
        return true;
    }

    template<typename T>
    bool batch_write_append(const T& val) {
        size_t len = sizeof(val);

        if ((buffer_.size() - (wwseq_ - wrseq_)) < len) {
            return false;
        }

        memcpy(&buffer_.data()[wwseq_ & mask_], &val, len);
        wwseq_ += len;
        return true;
    }

    void batch_write_commit() {
        wseq_ = wwseq_;
    }

    void batch_write_rollback() {
        wwseq_ = wseq_;
    }

    void batch_read_begin(const void** buf, size_t* len) {
        rwseq_ = wseq_;

        *buf = &buffer_.data()[rrseq_ & mask_];
        *len = rwseq_ - rrseq_;
    }

    void batch_read_commit() {
        rrseq_ = rwseq_;
        rseq_ = rwseq_;
    }

    void batch_read_commit(size_t len) {
        rrseq_ += len;
        rseq_ = rrseq_;
    }

    void batch_read_rollback() {
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

class spsc_batch_writer {
public:
    spsc_batch_writer(spsc_queue& queue)
        : queue_(queue)
        , done_(false)
    {
        queue_.batch_write_begin();
    }

    spsc_batch_writer(spsc_batch_writer&&) = delete;
    spsc_batch_writer(const spsc_batch_writer&) = delete;

    ~spsc_batch_writer() {
        if (!done_) {
            queue_.batch_write_rollback();
        }
    }

    spsc_batch_writer& operator=(spsc_batch_writer&&) = delete;
    spsc_batch_writer& operator=(const spsc_batch_writer&) = delete;

    template<typename T>
    void write(const T& val) {
        assert(!done_);
        if (!queue_.batch_write_append(val)) {
            throw std::runtime_error("write failed");
        }
    }

    void commit() {
        assert(!done_);
        queue_.batch_write_commit();
        done_ = true;
    }

private:
    spsc_queue& queue_;
    bool        done_;
};

class spsc_batch_reader {
public:
    spsc_batch_reader(spsc_queue& queue)
        : queue_(queue)
        , done_(false)
        , buf_(nullptr)
        , len_(0)
        , off_(0)
    {
        queue_.batch_read_begin(&buf_, &len_);
    }

    spsc_batch_reader(spsc_batch_reader&&) = delete;
    spsc_batch_reader(const spsc_batch_reader&) = delete;

    ~spsc_batch_reader() {
        if (!done_) {
            queue_.batch_read_rollback();
        }
    }

    spsc_batch_reader& operator=(spsc_batch_reader&&) = delete;
    spsc_batch_reader& operator=(const spsc_batch_reader&) = delete;

    template<typename T>
    bool try_read(T* val) {
        if ((len_ - off_) < sizeof(*val)) {
            return false;
        }

        memcpy(val, static_cast<const uint8_t*>(buf_)+off_, sizeof(*val));
        off_ += sizeof(*val);
        return true;
    }

    template<typename T>
    T read() {
        T val;
        if (!try_read(&val)) {
            throw std::runtime_error("read failed");
        }

        return val;
    }

    void commit() {
        assert(!done_);
        queue_.batch_read_commit(off_);
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

        for (int j = 0; j < (1 << 20); ++j) {
            {
                snw::spsc_batch_writer writer(queue);
                for (char c: "hello, world") {
                    writer.write(c);
                }
                writer.commit();
            }
            {
                snw::spsc_batch_reader reader(queue);

                std::vector<char> str;
                char c;
                while (reader.try_read(&c)) {
                    str.push_back(c);
                }
                str.push_back('\0');
                reader.commit();

                std::cout << str.data() << std::endl;
            }
        }
    }
    catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
    }

    return 0;
}
