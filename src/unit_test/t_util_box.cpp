#include "catch.hpp"
#include "box.h"

namespace {

struct counter {
	static int created_count;
    static int destroyed_count;
    static int copied_count;
    static int moved_count;

    counter() {
        ++created_count;
    }

    counter(counter&&) {
        ++moved_count;
    }

    counter(const counter&) {
        ++copied_count;
    }

    ~counter() {
        ++destroyed_count;
    }

    counter& operator=(counter&&) {
        ++moved_count;
    }

    counter& operator=(const counter&) {
        ++copied_count;
    }

    static void reset() {
        created_count = 0;
        destroyed_count = 0;
        copied_count = 0;
        moved_count = 0;
    }
};

int counter::created_count = 0;
int counter::destroyed_count = 0;
int counter::copied_count = 0;
int counter::moved_count = 0;

}

TEST_CASE("box") {
    SECTION("default construct a primitive") {
        snw::box<int> b;
        b.create();
        b.destroy();
    }

    SECTION("construct a primitive with an initial value") {
        snw::box<int> b;
        b.create(13);
        CHECK(*b == 13);
        b.destroy();
    }

    SECTION("an empty box") {
        snw::box<counter> b;
        (void)b;

        CHECK(counter::created_count == 0);
        CHECK(counter::destroyed_count == 0);
        counter::reset();
    }

    SECTION("a box that leaks should not call the destructor") {
        {
            snw::box<counter> b;
            b.create();
        }

        CHECK(counter::created_count == 1);
        CHECK(counter::moved_count == 0);
        CHECK(counter::destroyed_count == 0);
        counter::reset();
    }

    SECTION("destroy should call the destructor") {
        snw::box<counter> b;
        b.create();
        b.destroy();

        CHECK(counter::created_count == 1);
        CHECK(counter::destroyed_count == 1);
        counter::reset();
    }

    SECTION("release should end the object lifetime") {
        snw::box<counter> b;
        b.create();
        (void)b.release();

        CHECK(counter::created_count == 1);
        CHECK(counter::moved_count >= 1);
        CHECK(counter::destroyed_count == (counter::created_count + counter::moved_count));
        counter::reset();
    }
}
