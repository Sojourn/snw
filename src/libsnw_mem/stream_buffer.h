#pragma once

#include <cstdint>
#include <cstddef>

namespace snw {

class stream_buffer {
public:
    stream_buffer(size_t min_size);
    stream_buffer(stream_buffer&& other);
    stream_buffer(const stream_buffer&) = delete;
    ~stream_buffer();

    stream_buffer& operator=(stream_buffer&& rhs);
    stream_buffer& operator=(const stream_buffer&) = delete;

    explicit operator bool() const;
    void close();

    size_t size() const {
        return size_;
    }

    uint8_t* data() {
        return data_;
    }

    const uint8_t* data() const {
        return data_;
    }

private:
    uint8_t* data_;
    size_t   size_;
    int      fd_;
};

}
