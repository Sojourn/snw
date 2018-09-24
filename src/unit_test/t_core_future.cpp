#include "catch.hpp"
#include "future.h"

TEST_CASE("future") {
    SECTION("construct and assign") {
        snw::future_promise<int> fp;

        snw::future<int> f(std::move(fp.future));
        snw::promise<int> p(std::move(fp.promise));

        CHECK(fp.future.is_broken());
        CHECK(!f.is_broken());

        snw::future<int> f2;
        snw::promise<int> p2;

        f2 = std::move(f);
        p2 = std::move(p);

        CHECK(f.is_broken());
        CHECK(!f2.is_broken());
    }

    SECTION("states") {
        snw::future_promise<int> fp;
        auto f1 = std::move(fp.future);
        auto p1 = std::move(fp.promise);

        CHECK(!f1.is_broken());
        CHECK(f1.is_waiting());
        CHECK(!f1.has_value());

        // fufilling the promise should cause the future
        // value to become ready
        p1.set_value(3);
        CHECK(!f1.is_broken());
        CHECK(f1.has_value());
        CHECK(f1.value() == 3);

        // destroying a promise after the future is ready
        // shouldn't cause any state changes
        snw::promise<int>(std::move(p1));
        CHECK(!f1.is_broken());
        CHECK(f1.has_value());
        CHECK(f1.value() == 3);

        // moving a ready future should leave the original
        // future in a broken/no value state
        snw::future<int> f2(std::move(f1));
        CHECK(f1.is_broken());
        CHECK(!f1.has_value());
        CHECK(!f2.is_broken());
        CHECK(f2.has_value());
        CHECK(f2.value() == 3);

        // move assign for good measure
        snw::future<int> f3;
        f3 = std::move(f2);
        CHECK(f2.is_broken());
        CHECK(!f2.has_value());
        CHECK(!f3.is_broken());
        CHECK(f3.has_value());
        CHECK(f3.value() == 3);
    }

    SECTION("value lifetime") {
        // TODO
    }
}