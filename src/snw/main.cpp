#include <cstdint>

#include "intrusive_list.h"
#include "snw_lang.h"
#include "message_stream.h"
#include "slot_allocator.h"

int main(int argc, char** argv) {
    snw::slot_allocator sa(1 << 20);

    return 0;
}
