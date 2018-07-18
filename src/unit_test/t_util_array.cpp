#include "catch.hpp"
#include "array.h"
#include <utility>

TEST_CASE("array") {
    SECTION("default constructed") {
        snw::array<char, 16> array;

        CHECK(array.size() == 0);
        CHECK(array.capacity() == 16);
    }

    SECTION("empty") {
        snw::array<char, 3> array;

        CHECK(array.empty());
        array.push_back('a');
        CHECK(!array.empty());
    }

    SECTION("push_back") {
        snw::array<char, 3> array;

        CHECK(array.push_back('a'));
        CHECK(array.push_back('b'));
        CHECK(array.push_back('c'));
        CHECK(!array.push_back('d'));

        CHECK(array[0] == 'a');
        CHECK(array[1] == 'b');
        CHECK(array[2] == 'c');
    }

    SECTION("pop_back") {
        snw::array<char, 3> array;

        CHECK(array.push_back('a'));
        CHECK(array.push_back('b'));
        CHECK(array.push_back('c'));

        CHECK(array.size() == 3);
        CHECK(array[2] == 'c');

        array.pop_back();
        CHECK(array.size() == 2);
        CHECK(array[1] == 'b');

        array.pop_back();
        CHECK(array.size() == 1);
        CHECK(array[0] == 'a');

        array.pop_back();
        CHECK(array.size() == 0);

        array.pop_back(); // should have no effect
        CHECK(array.size() == 0);
    }

    SECTION("insert") {
        snw::array<char, 4> array;

        REQUIRE(array.insert(array.end(), 'a'));
        CHECK(array.size() == 1);
        CHECK(array[0] == 'a');

        REQUIRE(array.insert(array.end(), 'b'));
        CHECK(array.size() == 2);
        CHECK(array[0] == 'a');
        CHECK(array[1] == 'b');

        REQUIRE(array.insert(array.begin(), 'c'));
        CHECK(array.size() == 3);
        CHECK(array[0] == 'c');
        CHECK(array[1] == 'a');
        CHECK(array[2] == 'b');

        REQUIRE(array.insert(array.begin()+1, 'd'));
        CHECK(array.size() == 4);
        CHECK(array[0] == 'c');
        CHECK(array[1] == 'd');
        CHECK(array[2] == 'a');
        CHECK(array[3] == 'b');

        CHECK(!array.insert(array.begin(), 'e'));
    }

    SECTION("erase") {
        snw::array<char, 4> array;

        array.push_back('a');
        array.push_back('b');
        array.push_back('c');
        array.push_back('d');

        array.erase(array.end()); // should have no effect
        CHECK(array.size() == 4);

        array.erase(array.end()-1); // back
        CHECK(array.size() == 3);
        CHECK(array[0] == 'a');
        CHECK(array[1] == 'b');
        CHECK(array[2] == 'c');

        array.erase(array.begin()); // front
        CHECK(array.size() == 2);
        CHECK(array[0] == 'b');
        CHECK(array[1] == 'c');

        array.push_back('a');
        array.erase(array.begin()+1); // middle
        CHECK(array.size() == 2);
        CHECK(array[0] == 'b');
        CHECK(array[1] == 'a');
    }

    SECTION("move construct") {
        snw::array<char, 4> a1;
        {
            snw::array<char, 4> a2(std::move(a1));
            CHECK(a1.size() == 0);
            CHECK(a2.size() == 0);
        }
        {
            a1.push_back('a');
            a1.push_back('b');
            a1.push_back('c');

            snw::array<char, 4> a2(std::move(a1));
            CHECK(a1.size() == 0);
            CHECK(a2.size() == 3);
            CHECK(a2[0] == 'a');
            CHECK(a2[1] == 'b');
            CHECK(a2[2] == 'c');
        }
    }

    SECTION("move assign") {
        {
            snw::array<char, 4> a1;
            snw::array<char, 4> a2;
            CHECK(&(a2 = std::move(a1)) == &a2);
            CHECK(a1.size() == 0);
            CHECK(a2.size() == 0);
        }
        {
            snw::array<char, 4> a1;
            a1.push_back('a');
            a1.push_back('b');
            a1.push_back('c');

            snw::array<char, 4> a2;
            CHECK(&(a2 = std::move(a1)) == &a2);
            CHECK(a1.size() == 0);
            CHECK(a2.size() == 3);
            CHECK(a2[0] == 'a');
            CHECK(a2[1] == 'b');
            CHECK(a2[2] == 'c');
        }
    }
}
