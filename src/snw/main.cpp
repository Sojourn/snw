#include <iostream>
#include <vector>
#include <limits>
#include <string>
#include <atomic>
#include <memory>
#include <thread>
#include <stdexcept>
#include <cstdio>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cassert>

#include <unistd.h>

#include "snw_stream.h"
#include "align.h"
#include "varchar.h"

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

    for (int i = 0; i < (1 << 20); ++i) {
        stream.write<poke_msg>("me", "you");
        size_t cnt = stream.read([](const msg_base& base) {
            const poke_msg& msg = static_cast<const poke_msg&>(base);

            // std::cout << msg.sender << std::endl;
            // std::cout << msg.target << std::endl;
        });
        std::cout << cnt << std::endl;
    }

    return 0;
}
