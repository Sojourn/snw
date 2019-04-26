#include "catch.hpp"
#include "function.h"

struct copyable_arg {
    int value = -1;

    copyable_arg(int value): value(value) {}
};

struct movable_arg {
    int value = -1;

    movable_arg(int value): value(value) {}
    movable_arg(movable_arg&& other): value(other.value) { other.value = -1; }
    movable_arg(const movable_arg&) = delete;
    ~movable_arg() = default;

    movable_arg& operator=(movable_arg&& rhs) {
        if (this != &rhs) {
            value = rhs.value;
            rhs.value = -1;
        }
        return *this;
    }

    movable_arg& operator=(const movable_arg&) = delete;
};

struct foobar {
    int copyable(copyable_arg arg) {
        return arg.value;
    }

    int copyable_ref(copyable_arg& arg) {
        return arg.value;
    }

    int copyable_const_ref(const copyable_arg& arg) {
        return arg.value;
    }

    int movable(movable_arg arg) {
        return arg.value;
    }

    int movable_ref(movable_arg& arg) {
        return arg.value;
    }

    int movable_const_ref(const movable_arg& arg) {
        return arg.value;
    }
};

TEST_CASE("function") {
    SECTION("move construction and assignment") {
        snw::function<int()> f1;
        snw::function<int()> f2;

        f1 = std::move(f2);
        CHECK(!f1);
        CHECK(!f2);

        f1 = [](){ return 1; };
        CHECK(f1);

        f2 = std::move(f1);
        CHECK(f2);
        CHECK(!f1);

        CHECK(f2() == 1);
    }
    SECTION("copying and moving arguments") {
        foobar fb;

        // functors
        {
            snw::function<int(copyable_arg)> fn = [](copyable_arg arg) {
                return arg.value;
            };
            copyable_arg arg(13);
            CHECK(fn(arg) == 13);
        }
        {
            snw::function<int(copyable_arg&)> fn = [](copyable_arg& arg) {
                return arg.value;
            };
            copyable_arg arg(15);
            CHECK(fn(arg) == 15);
        }
        {
            snw::function<int(const copyable_arg&)> fn = [](const copyable_arg& arg) {
                return arg.value;
            };
            copyable_arg arg(17);
            CHECK(fn(arg) == 17);
        }
        {
            snw::function<int(movable_arg)> fn = [](movable_arg arg) {
                return arg.value;
            };
            movable_arg arg(19);
            CHECK(fn(std::move(arg)) == 19);
            CHECK(arg.value == -1);
        }
        {
            snw::function<int(movable_arg&)> fn = [](movable_arg& arg) {
                return arg.value;
            };
            movable_arg arg(21);
            CHECK(fn(arg) == 21);
        }
        {
            snw::function<int(const movable_arg&)> fn = [](const movable_arg& arg) {
                return arg.value;
            };
            movable_arg arg(23);
            CHECK(fn(arg) == 23);
        }

        // member functions
        {
            snw::function<int(copyable_arg)> fn(&fb, &foobar::copyable);
            copyable_arg arg(13);
            CHECK(fn(arg) == 13);
        }
        {
            snw::function<int(copyable_arg&)> fn(&fb, &foobar::copyable_ref);
            copyable_arg arg(15);
            CHECK(fn(arg) == 15);
        }
        {
            snw::function<int(const copyable_arg&)> fn(&fb, &foobar::copyable_const_ref);
            copyable_arg arg(17);
            CHECK(fn(arg) == 17);
        }
        {
            snw::function<int(movable_arg)> fn(&fb, &foobar::movable);
            movable_arg arg(19);
            CHECK(fn(std::move(arg)) == 19);
            CHECK(arg.value == -1);
        }
        {
            snw::function<int(movable_arg&)> fn(&fb, &foobar::movable_ref);
            movable_arg arg(21);
            CHECK(fn(arg) == 21);
        }
        {
            snw::function<int(const movable_arg&)> fn(&fb, &foobar::movable_const_ref);
            movable_arg arg(23);
            CHECK(fn(arg) == 23);
        }
    }
}
