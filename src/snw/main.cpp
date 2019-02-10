#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <bitset>
#include <memory>
#include <cstring>
#include <cstdint>
#include <cstddef>

#include "snw_util.h"
#include "snw_event.h"

#include "object.h"
#include "object_heap.h"

using namespace snw;

int main(int argc, char** argv) {
    object_heap heap;
    {
        auto nil = make_object<object_type::nil>(heap);
    }
    {
        auto t = make_object<object_type::boolean>(heap, true);
        auto f = make_object<object_type::boolean>(heap, false);
    }
    {
        auto i = make_object<object_type::integer>(heap, 13);
    }

    for (auto& root: heap.roots()) {
        std::cout << &root << std::endl;
    }

#ifdef SNW_OS_WINDOWS
    std::system("pause");
#endif
    return 0;
}
