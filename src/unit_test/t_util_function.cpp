#include "catch.hpp"
#include "function.h"

TEST_CASE("function") {
    SECTION("default construction") {
        snw::function<int(int)> fn;
    }
}