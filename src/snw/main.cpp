#include <iostream>
#include <cstdint>

#include "intrusive_list.h"
#include "snw_lang.h"
#include "message_stream.h"
#include "slot_allocator.h"

int main(int argc, char** argv) {
    snw::slot_allocator sa(1 << 20);

    for (int i = 0; i < 10; ++i) {
        uint64_t slot;
        if (sa.allocate(&slot)) {
            std::cout << "allocated: " << slot << std::endl;
        }
        else {
            std::cout << "allocation failed" << std::endl;
        }

        if (i && ((i % 7) == 0)) {
            sa.deallocate(slot - 1);
        }
    }

    sa.scan([&](uint64_t slot) {
        std::cout << "scanned: " << slot << std::endl;
    });

    return 0;
}
