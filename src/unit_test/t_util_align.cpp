#include "catch.hpp"
#include "align.h"
#include <vector>
#include <cstdint>

TEST_CASE("alignment utilities") {
    SECTION("align_up") {
        CHECK(snw::align_up(0, 16) == 0);
        CHECK(snw::align_up(1, 16) == 16);
        CHECK(snw::align_up(15, 16) == 16);
        CHECK(snw::align_up(16, 16) == 16);
        CHECK(snw::align_up(17, 16) == 32);
    }

    SECTION("align_down") {
        CHECK(snw::align_down(0, 16) == 0);
        CHECK(snw::align_down(1, 16) == 0);
        CHECK(snw::align_down(15, 16) == 0);
        CHECK(snw::align_down(16, 16) == 16);
        CHECK(snw::align_down(17, 16) == 16);
    }

    SECTION("is_aligned") {
        size_t max_alignment = 4096;

        std::vector<uint8_t> buffer;
        buffer.resize(max_alignment * 2);

        const uint8_t* aligned_base = snw::align_up(buffer.data(), max_alignment);
        REQUIRE(buffer.data() <= aligned_base);
        REQUIRE(aligned_base < (buffer.data() + buffer.size()));

        for (int alignment = 1; alignment <= max_alignment; alignment = alignment << 1) {
            for (int i = 0; i < max_alignment; ++i) {
                // test for integer arguments
                CHECK(snw::is_aligned(i, alignment) != static_cast<bool>(i % alignment));

                // test for pointer arguments
                CHECK(snw::is_aligned(aligned_base + i, alignment) != static_cast<bool>(i % alignment));
            }
        }
    }
}
