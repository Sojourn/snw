#include <iostream>
#include <vector>
#include <limits>
#include <string>
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

}

int main(int argc, const char** argv) {
    try {
        snw::message_stream stream(1 << 20);

        stream.write("hello");
        stream.write("world");

        while (const char* msg = stream.read()) {
            std::cout << msg << std::endl;
        }
    }
    catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
    }

    return 0;
}
