#include "catch.hpp"
#include "text_reader.h"

TEST_CASE("text_reader") {
    SECTION("getc") {
        snw::text_reader<0> reader("abcd edf\n");

        CHECK(reader.getc() == 'a');
        CHECK(reader.getc() == 'b');
        CHECK(reader.getc() == 'c');
        CHECK(reader.getc() == 'd');
        CHECK(reader.getc() == ' ');
        CHECK(reader.getc() == 'e');
        CHECK(reader.getc() == 'd');
        CHECK(reader.getc() == 'f');
        CHECK(reader.getc() == '\n');
    }
    SECTION("ungetc-1") {
        snw::text_reader<1> reader("abcd edf\n");

        CHECK(reader.getc() == 'a');
        CHECK(reader.getc() == 'b');
        CHECK(reader.getc() == 'c');
        reader.ungetc();
        CHECK(reader.getc() == 'c');
        CHECK(reader.getc() == 'd');
        CHECK(reader.getc() == ' ');
        reader.ungetc();
        CHECK(reader.getc() == ' ');
        CHECK(reader.getc() == 'e');
        CHECK(reader.getc() == 'd');
        reader.ungetc();
        CHECK(reader.getc() == 'd');
        CHECK(reader.getc() == 'f');
        CHECK(reader.getc() == '\n');
    }
    SECTION("ungetc-2") {
        snw::text_reader<2> reader("abcd edf\n");

        CHECK(reader.getc() == 'a');
        CHECK(reader.getc() == 'b');
        CHECK(reader.getc() == 'c');
        reader.ungetc();
        reader.ungetc();
        CHECK(reader.getc() == 'b');
        CHECK(reader.getc() == 'c');
        CHECK(reader.getc() == 'd');
        CHECK(reader.getc() == ' ');
        reader.ungetc();
        reader.ungetc();
        CHECK(reader.getc() == 'd');
        CHECK(reader.getc() == ' ');
        CHECK(reader.getc() == 'e');
        CHECK(reader.getc() == 'd');
        reader.ungetc();
        reader.ungetc();
        CHECK(reader.getc() == 'e');
        CHECK(reader.getc() == 'd');
        CHECK(reader.getc() == 'f');
        CHECK(reader.getc() == '\n');
    }
    SECTION("offset") {
        snw::text_reader<2> reader("abcd edf\n");
        for (size_t i = 0; reader.getc(); ++i) {
            CHECK(i == reader.offset());
        }
    }
    SECTION("column counts") {
        // TODO
    }
    SECTION("row counts") {
        // TODO
    }
}
