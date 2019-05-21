#include <iostream>
#include <cstdint>

#include "intrusive_list.h"
#include "snw_lang.h"
#include "message_stream.h"
#include "slot_allocator.h"

int main(int argc, char** argv) {
    snw::slot_allocator sa(1 << 20);

    {
        uint64_t slot;
        if (sa.allocate(&slot)) {
            std::cout << slot << std::endl;
        }
        else {
            std::cout << "allocation failed" << std::endl;
        }
    }

    return 0;
}
