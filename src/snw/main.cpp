#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <bitset>
#include <memory>
#include <atomic>
#include <cstring>
#include <cstdint>
#include <cstddef>

#include "snw_util.h"
#include "snw_event.h"

#include "object.h"
#include "object_heap.h"
#include "object_stack.h"
#include "object_transaction.h"

using namespace snw;

int main(int argc, char** argv) {
    uint8_t buf[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };

    object_heap heap;
    {
        auto n = heap.new_nil();
        auto b0 = heap.new_boolean(false);
        auto b1 = heap.new_boolean(true);
        auto sym = heap.new_symbol("main");
        auto str0 = heap.new_string("");
        auto str1 = heap.new_string("Hello, World!");
        auto buf0 = heap.new_bytes(nullptr, nullptr);
        auto buf1 = heap.new_bytes(buf, buf + sizeof(buf));
        auto cell0 = heap.new_cell(n, n);
        auto cell1 = heap.new_cell(sym, cell0);
        auto cell2 = heap.new_cell(str1, cell1);

        {
            object_ref refs[0] = {
            };

            auto empty_list = heap.new_list(refs, refs + 0);
        }
        {
            object_ref refs[3] = {
                heap.new_symbol("a"),
                heap.new_symbol("b"),
                heap.new_symbol("c"),
            };

            auto list = heap.new_list(refs, refs + 3);
        }

        // std::cout << heap.deref_nil(n) << std::endl;
        std::cout << heap.deref_boolean(b0) << std::endl;
        std::cout << heap.deref_boolean(b1) << std::endl;
        std::cout << heap.deref_symbol(sym) << std::endl;
        // std::cout << heap.deref_string(str0) << std::endl;
        // std::cout << heap.deref_string(str1) << std::endl;
        // std::cout << heap.deref_bytes(buf0) << std::endl;
        // std::cout << heap.deref_bytes(buf1) << std::endl;
        // std::cout << heap.deref_cell(cell0) << std::endl;
        // std::cout << heap.deref_cell(cell1) << std::endl;
        // std::cout << heap.deref_cell(cell2) << std::endl;
    }

#ifdef SNW_OS_WINDOWS
    std::system("pause");
#endif
    return 0;
}
