#include "catch.hpp"
#include "stream_buffer.h"
#include <cstring>

TEST_CASE("stream_buffer") {
    SECTION("construction and assignment") {
        {
            snw::stream_buffer sb(0);
            CHECK(sb);
            CHECK(sb.size() > 0);
        }
        {
            snw::stream_buffer sb(1);
            CHECK(sb);
            CHECK(sb.size() > 0);
        }
        {
            snw::stream_buffer sb1(1);
            snw::stream_buffer sb2(std::move(sb1));
            CHECK(!sb1);
            CHECK(sb2);
            CHECK(sb2.size() > 0);

            sb1 = std::move(sb2);
            CHECK(sb1);
            CHECK(sb1.size() > 0);
            CHECK(!sb2);
        }
    }
    SECTION("reflection") {
        snw::stream_buffer sb(4096);
        uint8_t* lower_data = sb.data();
        uint8_t* upper_data = sb.data() + sb.size();

        for (size_t i = 0; i < sb.size(); ++i) {
            lower_data[i] = static_cast<uint8_t>(i);
            CHECK(lower_data[i] == upper_data[i]);
        }

        memset(lower_data, 0, sb.size());
        memset(upper_data, 0, sb.size());

        for (size_t i = 0; i < sb.size(); ++i) {
            upper_data[i] = static_cast<uint8_t>(i);
            CHECK(lower_data[i] == upper_data[i]);
        }
    }
    SECTION("wrapping") {
        snw::stream_buffer sb(4096);
        uint8_t* lower_data = sb.data();
        uint8_t* upper_data = sb.data() + sb.size();

        memset(lower_data + sb.size() - 4, 'A', 8);

        CHECK(memcmp(lower_data + sb.size() - 4, lower_data, 4) == 0);
        CHECK(memcmp(upper_data + sb.size() - 4, upper_data, 4) == 0);
    }
}
