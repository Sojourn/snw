#pragma once

#include <stdexcept>
#include <cstring>
#include <cassert>
#include "align.h"
#include "message_stream.h"

template<typename MessageBase, typename Stream>
snw::basic_message_stream<MessageBase, Stream>::basic_message_stream(size_t min_size)
    : stream_(min_size)
{
}

template<typename MessageBase, typename Stream>
template<typename MessageHandler>
size_t snw::basic_message_stream<MessageBase, Stream>::read(MessageHandler&& handler, size_t max_cnt) {
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
            void* ptr = stream_.read(len);
            assert(ptr);

            MessageBase& message = *reinterpret_cast<MessageBase*>(ptr);
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

template<typename MessageBase, typename Stream>
template<typename Message, typename... Args>
bool snw::basic_message_stream<MessageBase, Stream>::try_write(Args&&... args) {
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

template<typename MessageBase, typename Stream>
template<typename Message, typename... Args>
void snw::basic_message_stream<MessageBase, Stream>::write(Args&&... args) {
    if (!try_write<Message>(std::forward<Args>(args)...)) {
        throw std::runtime_error("write failed");
    }
}
