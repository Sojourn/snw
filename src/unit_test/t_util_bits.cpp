#include "catch.hpp"
#include "bits.h"

TEST_CASE("bit utilities") {
    SECTION("count_trailing_zeros") {
        for (int i = 0; i < 32; ++i) {
            CHECK(snw::count_trailing_zeros(static_cast<uint32_t>(1ull << i)) == i);
            CHECK(snw::count_trailing_zeros(static_cast<uint64_t>(1ull << i)) == i);
        }
        for (int i = 32; i < 64; ++i) {
            CHECK(snw::count_trailing_zeros(static_cast<uint64_t>(1ull << i)) == i);
        }
    }
}
