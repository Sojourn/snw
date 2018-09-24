#include <iostream>
#include <string>
#include <vector>

#include "snw_util.h"
#include "snw_core.h"

namespace snw {
    // alias this until it gets renamed
    template<template<typename> class... Modules>
    using event_router = core<Modules...>;
}

int main(int argc, char** argv) {

#ifdef SNW_OS_WINDOWS
    std::system("pause");
#endif
    return 0;
}
