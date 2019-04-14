#include <iostream>
#include <vector>
#include <limits>
#include <string>
#include <atomic>
#include <memory>
#include <stdexcept>
#include <cstdio>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cassert>

#include <unistd.h>

#include "stream_buffer.h"
#include "align.h"
#include "varchar.h"

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

template<typename MessageBase, typename Stream>
class basic_message_stream {
public:
    basic_message_stream(size_t min_size)
        : stream_(min_size)
    {
    }

    template<typename MessageHandler>
    size_t read(MessageHandler&& handler, size_t max_cnt = 0) {
        stream_.read_begin();

        size_t cnt = 0;
        for (; cnt <= (max_cnt - 1); ++cnt) {
            size_t len;
            {
                const void* ptr = stream_.template read<sizeof(len)>();
                if (!ptr) {
                    stream_.read_rollback();
                    return cnt;
                }

                memcpy(&len, ptr, sizeof(len));
            }

            {
                const void* ptr = stream_.read(len);
                assert(ptr);

                try {
                    const MessageBase& message = *reinterpret_cast<const MessageBase*>(ptr);
                    handler(message);
                    message.~MessageBase();
                }
                catch (const std::exception &) {
                    // FIXME: do we want to rollback or commit?
                    stream_.read_rollback();
                    throw;
                }
            }
        }

        stream_.read_commit();
        return cnt;
    }

    template<typename Message, typename... Args>
    bool try_write(Args&&... args) {
        stream_.write_begin();

        // write message length
        {
            size_t len = sizeof(size_t) + align_up(sizeof(Message), alignof(size_t));
            void* ptr = stream_.write<sizeof(len)>();
            if (!ptr) {
                stream_.write_rollback();
                return false;
            }

            memcpy(ptr, &len, sizeof(len));
        }

        // write message
        {
            void* ptr = stream_.write<sizeof(Message)>();
            if (!ptr) {
                stream_.write_rollback();
                return false;
            }

            new(ptr) Message(std::forward<Args>(args)...);
        }

        stream_.write_commit();
        return true;
    }

    template<typename Message, typename... Args>
    void write(Args&&... args) {
        if (!try_write<Message>(std::forward<Args>(args)...)) {
            throw std::runtime_error("write failed");
        }
    }

private:
    basic_message_stream(basic_message_stream&&) = delete;
    basic_message_stream(const basic_message_stream&) = delete;
    basic_message_stream& operator=(basic_message_stream&&) = delete;
    basic_message_stream& operator=(const basic_message_stream&) = delete;

    Stream stream_;
};

template<typename MessageBase>
using message_stream = basic_message_stream<MessageBase, byte_stream>;

template<typename MessageBase>
using atomic_message_stream = basic_message_stream<MessageBase, atomic_byte_stream>;

}

struct msg_base {
    virtual ~msg_base() {}
};

struct poke_msg : public msg_base {
    snw::varchar<16>     sender;
    snw::varchar<16>     target;
    std::vector<uint8_t> appendage;

    poke_msg(const char* sender, const char* target)
        : sender(sender)
        , target(target)
    {}

    virtual ~poke_msg() {}
};

int main(int argc, const char** argv) {
    snw::message_stream<msg_base> stream(1 << 16);

    stream.write<poke_msg>("me", "you");
    size_t cnt = stream.read([](const msg_base& base) {
        const poke_msg& msg = static_cast<const poke_msg&>(base);

        std::cout << msg.sender << std::endl;
        std::cout << msg.target << std::endl;
    });
    std::cout << cnt << std::endl;

    return 0;
}
