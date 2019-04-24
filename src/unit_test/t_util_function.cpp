#include "catch.hpp"
#include "function.h"

TEST_CASE("function") {
    SECTION("default construction") {
        snw::basic_function<64, int(int)> fn;
    }
}