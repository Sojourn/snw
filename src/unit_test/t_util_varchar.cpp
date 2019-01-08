#include "catch.hpp"
#include "varchar.h"

TEST_CASE("varchar") {
    SECTION("construct and assign") {
        snw::varchar<16> s;
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

        s = snw::varchar<16>("12");
        CHECK(!s.empty());
        CHECK(s.size() == 2);
        CHECK(s[0] == '1');
        CHECK(s[1] == '2');
    }

    SECTION("comparisons") {
        CHECK(snw::varchar<16>("") == snw::varchar<16>(""));
        CHECK(snw::varchar<16>("a") == snw::varchar<16>("a"));
        CHECK(snw::varchar<16>("ab") == snw::varchar<16>("ab"));

        CHECK(snw::varchar<16>("") >= snw::varchar<16>(""));
        CHECK(snw::varchar<16>("a") >= snw::varchar<16>("a"));
        CHECK(snw::varchar<16>("ab") >= snw::varchar<16>("ab"));

        CHECK(snw::varchar<16>("") <= snw::varchar<16>(""));
        CHECK(snw::varchar<16>("a") <= snw::varchar<16>("a"));
        CHECK(snw::varchar<16>("ab") <= snw::varchar<16>("ab"));

        CHECK(snw::varchar<16>("b") != snw::varchar<16>("a"));
        CHECK(snw::varchar<16>("ab") != snw::varchar<16>("aa"));

        CHECK(snw::varchar<16>("b") > snw::varchar<16>("a"));
        CHECK(snw::varchar<16>("ab") > snw::varchar<16>("aa"));

        CHECK(snw::varchar<16>("b") >= snw::varchar<16>("a"));
        CHECK(snw::varchar<16>("ab") >= snw::varchar<16>("aa"));

        CHECK(snw::varchar<16>("a") < snw::varchar<16>("b"));
        CHECK(snw::varchar<16>("aa") < snw::varchar<16>("ab"));

        CHECK(snw::varchar<16>("a") <= snw::varchar<16>("b"));
        CHECK(snw::varchar<16>("aa") <= snw::varchar<16>("ab"));
    }
}
