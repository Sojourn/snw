#pragma once

#include <stdexcept>
#include <cstring>
#include <cstdint>
#include <cstddef>
#include <cassert>
#include "byte_stream.h"
#include "align.h"

namespace snw {

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

        // special loop bounds to make max_cnt==0 act like max_cnt==infinity
        size_t cnt = 0;
        for (; cnt <= (max_cnt - 1); ++cnt) {
            size_t len;
            {
                const void* ptr = stream_.template read<sizeof(len)>();
                if (!ptr) {
                    break;
                }

                memcpy(&len, ptr, sizeof(len));
            }

            {
                const void* ptr = stream_.read(len);
                assert(ptr);

                const MessageBase& message = *reinterpret_cast<const MessageBase*>(ptr);
                try {
                    handler(message);
                    message.~MessageBase(); // better not throw...
                }
                catch (const std::exception &) {
                    message.~MessageBase(); // better not throw...
                    stream_.read_commit();
                    throw;
                }
            }
        }

        stream_.read_commit();
        return cnt;
    }

    template<typename Message, typename... Args>
    bool try_write(Args&&... args) {
        static constexpr size_t msg_len = align_up(sizeof(Message), alignof(size_t));

        stream_.write_begin();

        // write message length
        {
            size_t len = msg_len;
            void* ptr = stream_.write<sizeof(len)>();
            if (!ptr) {
                stream_.write_rollback();
                return false;
            }

            memcpy(ptr, &len, sizeof(len));
        }

        // write message
        {
            void* ptr = stream_.write<msg_len>();
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
