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
    {
        auto s = make_object<object_type::symbol>(heap, "main");

        std::cout << s.value() << std::endl;
    }
    {
        auto s = make_object<object_type::string>(heap, "hello, world!");
        std::cout << s.c_str() << std::endl;
    }
    {
        auto c = make_object<object_type::cell>(heap);
        c.set_car(make_object<object_type::integer>(heap, 3));
        c.set_cdl(make_object<object_type::nil>(heap));
    }

    for (auto& root: heap.roots()) {
        std::cout << &root << std::endl;
    }

#ifdef SNW_OS_WINDOWS
    std::system("pause");
#endif
    return 0;
}
