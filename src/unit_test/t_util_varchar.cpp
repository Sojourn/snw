#include "catch.hpp"
#include "varchar.h"

TEST_CASE("varchar") {
    SECTION("construct and assign") {
        snw::varchar<4> s;
        CHECK(s.empty());
        CHECK(s.size() == 0);

        s.assign("abc");
        CHECK(!s.empty());
        CHECK(s.size() == 3);
        CHECK(s[0] == 'a');
        CHECK(s[1] == 'b');
        CHECK(s[2] == 'c');

        s = "abcd";
        CHECK(!s.empty());
        CHECK(s.size() == 4);
        CHECK(s[0] == 'a');
        CHECK(s[1] == 'b');
        CHECK(s[2] == 'c');
        CHECK(s[3] == 'd');

        s = snw::varchar<4>("12");
        CHECK(!s.empty());
        CHECK(s.size() == 2);
        CHECK(s[0] == '1');
        CHECK(s[1] == '2');
    }

    SECTION("comparisons") {
        CHECK(snw::varchar<2>("") == snw::varchar<2>(""));
        CHECK(snw::varchar<2>("a") == snw::varchar<2>("a"));
        CHECK(snw::varchar<2>("ab") == snw::varchar<2>("ab"));

        CHECK(snw::varchar<2>("") >= snw::varchar<2>(""));
        CHECK(snw::varchar<2>("a") >= snw::varchar<2>("a"));
        CHECK(snw::varchar<2>("ab") >= snw::varchar<2>("ab"));

        CHECK(snw::varchar<2>("") <= snw::varchar<2>(""));
        CHECK(snw::varchar<2>("a") <= snw::varchar<2>("a"));
        CHECK(snw::varchar<2>("ab") <= snw::varchar<2>("ab"));

        CHECK(snw::varchar<2>("b") != snw::varchar<2>("a"));
        CHECK(snw::varchar<2>("ab") != snw::varchar<2>("aa"));

        CHECK(snw::varchar<2>("b") > snw::varchar<2>("a"));
        CHECK(snw::varchar<2>("ab") > snw::varchar<2>("aa"));

        CHECK(snw::varchar<2>("b") >= snw::varchar<2>("a"));
        CHECK(snw::varchar<2>("ab") >= snw::varchar<2>("aa"));

        CHECK(snw::varchar<2>("a") < snw::varchar<2>("b"));
        CHECK(snw::varchar<2>("aa") < snw::varchar<2>("ab"));

        CHECK(snw::varchar<2>("a") <= snw::varchar<2>("b"));
        CHECK(snw::varchar<2>("aa") <= snw::varchar<2>("ab"));
    }
}
