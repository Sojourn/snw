#include "catch.hpp"
#include "lexer.h"
#include <vector>
#include <string>
#include <sstream>

enum class token_type {
    open_file,
    close_file,
    open_list,
    close_list,
    integer,
    string,
    symbol,
    comment,
    error,
};

using token = std::pair<token_type, std::string>;

struct token_collector {
    std::vector<token> tokens;

    void open_file() {
        tokens.push_back(std::make_pair(token_type::open_file, ""));
    }

    void close_file() {
        tokens.push_back(std::make_pair(token_type::close_file, ""));
    }

    void open_list() {
        tokens.push_back(std::make_pair(token_type::open_list, ""));
    }

    void close_list() {
        tokens.push_back(std::make_pair(token_type::close_list, ""));
    }

    void integer(int64_t value) {
        std::stringstream ss;
        ss << value;
        tokens.push_back(std::make_pair(token_type::integer, ss.str()));
    }

    void string(const char* first, const char* last) {
        tokens.push_back(std::make_pair(token_type::string, std::string(first, last-first)));
    }

    void symbol(const char* first, const char* last) {
        tokens.push_back(std::make_pair(token_type::symbol, std::string(first, last-first)));
    }

    void comment(const char* first, const char* last) {
        tokens.push_back(std::make_pair(token_type::comment, std::string(first, last-first)));
    }

    void error(const snw::lexer_error&) {
        tokens.push_back(std::make_pair(token_type::error, ""));
    }
};

std::vector<token> tokenize(const char* str) {
    token_collector collector;
    snw::parse(str, collector);
    return collector.tokens;
}

TEST_CASE("lexer") {
    SECTION("empty") {
        std::vector<token> tokens;
        tokens.push_back(std::make_pair(token_type::open_file, ""));
        tokens.push_back(std::make_pair(token_type::close_file, ""));

        CHECK(tokenize("") == tokens);
    }
    SECTION("nested lists") {
        std::vector<token> tokens;
        tokens.push_back(std::make_pair(token_type::open_file, ""));
        tokens.push_back(std::make_pair(token_type::open_list, ""));
        tokens.push_back(std::make_pair(token_type::open_list, ""));
        tokens.push_back(std::make_pair(token_type::open_list, ""));
        tokens.push_back(std::make_pair(token_type::close_list, ""));
        tokens.push_back(std::make_pair(token_type::close_list, ""));
        tokens.push_back(std::make_pair(token_type::close_list, ""));
        tokens.push_back(std::make_pair(token_type::close_file, ""));

        CHECK(tokenize("((()))") == tokens);
    }
    SECTION("list of integers") {
        std::vector<token> tokens;
        tokens.push_back(std::make_pair(token_type::open_file, ""));
        tokens.push_back(std::make_pair(token_type::open_list, ""));
        tokens.push_back(std::make_pair(token_type::integer, "1"));
        tokens.push_back(std::make_pair(token_type::integer, "2"));
        tokens.push_back(std::make_pair(token_type::integer, "3"));
        tokens.push_back(std::make_pair(token_type::close_list, ""));
        tokens.push_back(std::make_pair(token_type::close_file, ""));

        CHECK(tokenize("(1 2 3)") == tokens);
    }
    SECTION("list of symbols") {
        std::vector<token> tokens;
        tokens.push_back(std::make_pair(token_type::open_file, ""));
        tokens.push_back(std::make_pair(token_type::open_list, ""));
        tokens.push_back(std::make_pair(token_type::symbol, "+"));
        tokens.push_back(std::make_pair(token_type::symbol, "apples"));
        tokens.push_back(std::make_pair(token_type::symbol, "is-even?"));
        tokens.push_back(std::make_pair(token_type::close_list, ""));
        tokens.push_back(std::make_pair(token_type::close_file, ""));

        CHECK(tokenize("(+ apples is-even?)") == tokens);
    }
    SECTION("list of strings") {
        std::vector<token> tokens;
        tokens.push_back(std::make_pair(token_type::open_file, ""));
        tokens.push_back(std::make_pair(token_type::open_list, ""));
        tokens.push_back(std::make_pair(token_type::string, "+"));
        tokens.push_back(std::make_pair(token_type::string, "apples"));
        tokens.push_back(std::make_pair(token_type::string, "is-even?"));
        tokens.push_back(std::make_pair(token_type::close_list, ""));
        tokens.push_back(std::make_pair(token_type::close_file, ""));

        CHECK(tokenize("(\"+\" \"apples\" \"is-even?\")") == tokens);
    }
}
