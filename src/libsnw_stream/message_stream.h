#pragma once

#include <cstddef>
#include "byte_stream.h"

namespace snw {

template<typename MessageBase, typename Stream>
class basic_message_stream {
public:
    basic_message_stream(size_t min_size);
    basic_message_stream(basic_message_stream&&) = delete;
    basic_message_stream(const basic_message_stream&) = delete;

    basic_message_stream& operator=(basic_message_stream&&) = delete;
    basic_message_stream& operator=(const basic_message_stream&) = delete;

    template<typename MessageHandler>
    size_t read(MessageHandler&& handler, size_t max_cnt = 0);

    template<typename Message, typename... Args>
    bool try_write(Args&&... args);

    template<typename Message, typename... Args>
    void write(Args&&... args);

private:
    Stream stream_;
};

template<typename MessageBase>
using message_stream = basic_message_stream<MessageBase, byte_stream>;

template<typename MessageBase>
using atomic_message_stream = basic_message_stream<MessageBase, atomic_byte_stream>;

}

#include "message_stream.hpp"
