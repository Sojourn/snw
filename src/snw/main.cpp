#include <iostream>
#include <string>
#include <map>
#include <unordered_map>
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
#include "snw_lang.h"

using namespace snw;

class list_builder {
public:
    list_builder(object_heap& heap)
        : heap_(heap)
    {
    }

    void push_back(object_ref ref) {
        refs_.push_back(ref);
    }

    object_ref make_list() {
        if (refs_.empty()) {
            return heap_.new_nil();
        }

        auto first = refs_.data();
        auto last = first + refs_.size();
        return heap_.new_list(first, last);
    }

private:
    object_heap&           heap_;
    array<object_ref, 128> refs_;
};

struct process;
using function = void(*)(process&);
using environment = std::map<symbol, function>;

class process {
public:
    process() = default;

    object_ref eval(object_ref ref) {
        return ref;
    }

private:
    object_heap  heap_;
    environment  env_;
};

int main(int argc, char** argv) {
    process proc;
    // auto result = proc.eval();
    // print(std::cout, proc.heap, result);

    // object_heap heap;
    // object_ref program = parser(heap).parse("(+ 1 2)\n(divide (+ 3 4) 7)\n(\"Hello\tWorld\")");
    // print(std::cout, heap, program);

#ifdef SNW_OS_WINDOWS
    std::system("pause");
#endif
    return 0;
}
