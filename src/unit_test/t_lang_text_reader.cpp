#include "catch.hpp"
#include "text_reader.h"

TEST_CASE("text_reader") {
    SECTION("getc") {
        snw::text_reader<1> reader("abcd edf\n");

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
        snw::text_reader<2> reader("abcd edf\n");

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
        snw::text_reader<4> reader("abcd edf\n");

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
        snw::text_reader<1> reader("12\n\t34\r56");

        CHECK(reader.getc() == '1');
        CHECK(reader.getc() == '2');
        CHECK(reader.getc() == '\n');
        CHECK(reader.getc() == '\t');
        CHECK(reader.getc() == '3');
        CHECK(reader.getc() == '4');
        CHECK(reader.getc() == '\r');
        CHECK(reader.getc() == '5');
        CHECK(reader.getc() == '6');
    }
    SECTION("row counts") {
        snw::text_reader<1> reader("12\n\t34\r56");

        CHECK(reader.getc() == '1');
        CHECK(reader.row() == 0);
        CHECK(reader.getc() == '2');
        CHECK(reader.row() == 0);
        CHECK(reader.getc() == '\n');
        CHECK(reader.row() == 1);
        CHECK(reader.getc() == '\t');
        CHECK(reader.row() == 1);
        CHECK(reader.getc() == '3');
        CHECK(reader.row() == 1);
        CHECK(reader.getc() == '4');
        CHECK(reader.row() == 1);
        CHECK(reader.getc() == '\r');
        CHECK(reader.row() == 1);
        CHECK(reader.getc() == '5');
        CHECK(reader.row() == 1);
        CHECK(reader.getc() == '6');
        CHECK(reader.row() == 1);
    }
}
