#include <iostream>
#include <memory>
#include <string>
#include <cstdint>

#include "intrusive_list.h"
#include "snw_lang.h"
#include "message_stream.h"
#include "slot_allocator.h"
#include "snw_io.h"

int main(int argc, char** argv) {
    try {
        // snw::address addr("google.com", snw::socket_address_family::ipv4);
        snw::address addr("google.com");
        addr.set_port(80);

        std::cout << addr.to_string() << std::endl;
    }
    catch (const std::exception& ex) {
        std::cout << ex.what() << std::endl;
    }

#if defined(SNW_OS_WINDOWS)
    std::system("pause");
#endif
    return 0;
}
