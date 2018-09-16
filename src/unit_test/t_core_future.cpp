#include "catch.hpp"
#include "future.h"

TEST_CASE("future") {
    SECTION("construct and assign") {
        snw::future<int> f;
        snw::promise<int> p;
    }
}