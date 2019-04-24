#include <iostream>
#include <chrono>
#include <vector>
#include <limits>
#include <string>
#include <atomic>
#include <memory>
#include <thread>
#include <stdexcept>
#include <type_traits>
#include <cstdio>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cassert>

#include <unistd.h>

#include "snw_stream.h"
#include "align.h"
#include "varchar.h"
#include "intrusive_list.h"
#include "function.h"

struct copyable_arg {
};

struct movable_arg {
    movable_arg() = default;
    movable_arg(movable_arg&&) = default;
    movable_arg(const movable_arg&) = delete;
    ~movable_arg() = default;
    movable_arg& operator=(movable_arg&&) = default;
    movable_arg& operator=(const movable_arg&) = delete;
};

struct foo {
    char a() {
        return 'a';
    }

    char b(copyable_arg) {
        return 'b';
    }

    char c(movable_arg) {
        return 'c';
    }
};

int main(int argc, const char** argv) {
    foo f;
    copyable_arg carg;
    movable_arg marg;

    {
        snw::basic_function<64, char()> fn(&f, &foo::a);
        std::cout << fn() << std::endl;
    }
    {
        snw::basic_function<64, char(copyable_arg)> fn(&f, &foo::b);
        std::cout << fn(carg) << std::endl;
    }
    {
        snw::basic_function<64, char(movable_arg)> fn(&f, &foo::c);
        std::cout << fn(std::move(marg)) << std::endl;
    }

    return 0;
}
