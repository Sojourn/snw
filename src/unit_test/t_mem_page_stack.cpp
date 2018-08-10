#include "catch.hpp"
#include "page_stack.h"

TEST_CASE("page_stack") {
    SECTION("default constructed") {
        snw::page_stack page_stack;

        REQUIRE(page_stack.empty());
    }

    SECTION("push and pop") {
        // TODO: Test pushing and popping nodes (i.e. a lot of pages)
    }

    SECTION("move construct") {
        snw::page pages[8];

        snw::page_stack page_stack1;
        for(snw::page& page: pages) {
            page_stack1.push_back(&page);
        }

        REQUIRE(!page_stack1.empty());

        snw::page_stack page_stack2(std::move(page_stack1));

        REQUIRE(page_stack1.empty());
        REQUIRE(!page_stack2.empty());

        for(int i = 7; i >= 0; --i) {
            REQUIRE(page_stack2.pop_back() == &pages[i]);
        }

        REQUIRE(page_stack2.empty());
    }

    SECTION("move assign") {
        snw::page pages[8];

        snw::page_stack page_stack1;
        for(snw::page& page: pages) {
            page_stack1.push_back(&page);
        }

        REQUIRE(!page_stack1.empty());

        snw::page_stack page_stack2;
        page_stack2 = std::move(page_stack1);

        REQUIRE(page_stack1.empty());
        REQUIRE(!page_stack2.empty());

        for(int i = 7; i >= 0; --i) {
            REQUIRE(page_stack2.pop_back() == &pages[i]);
        }

        REQUIRE(page_stack2.empty());
    }
}