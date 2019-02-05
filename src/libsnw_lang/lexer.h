#pragma once

#include <stdexcept>
#include <string>
#include <cstring>
#include <cstddef>
#include <cstdint>
#include <cassert>

namespace snw {

    // struct token {
    //     size_t off;
    //     size_t len;
    //     size_t row;
    //     size_t col;
    // };

    inline bool is_digit(char c) {
        return ('0' <= c) && (c <= '9');
    }

    inline bool is_valid_string_char(char c) {
        // TODO: printable
        return true;
    }

    inline bool is_escapable_char(char c) {
        switch (c) {
        case '"':
        case '\\':
        case 't':
        case 'n':
        case 'r':
            return true;

        default:
            return false;
        }
    }

    inline char unescape(char c) {
        switch (c) {
        case 't':
            return '\t';
        case 'n':
            return '\n';
        case 'r':
            return '\r';

        default:
            return c;
        }
    }

    inline bool is_valid_symbol_char(char c) {
        switch (c) {
        case '_':
        case '-':
        case '+':
        case '/':
        case '*':
        case '%':
        case '=':
        case '!':
        case '^':
        case '&':
        case '<':
        case '>':
            return true;

        default:
            if (('a' <= c) && (c <= 'z')) {
                return true;
            }
            else if (('A' <= c) && (c <= 'Z')) {
                return true;
            }
            else {
                return false;
            }
        }
    }

    class lexer {
    public:
        lexer(const char* src) {
            reset(src);
        }

        template<typename Observer>
        void next(Observer&& observer) {
            observer.open_file();

            while (const char& c = getc()) {
                switch (c) {
                // whitespace
                case ' ':
                case '\t':
                case '\r':
                case '\n':
                    continue;

                // line comment
                case ';':
                    while (const char& c = getc()) {
                        if (c == '\n') {
                            break;
                        }
                    }
                    continue;

                case '(':
                    observer.open_list();
                    break;

                case ')':
                    observer.close_list();
                    break;

                case '"': {
                    const char* first = nullptr;
                    const char* last = nullptr;

                    while (const char& c = getc()) {
                    }

                    break;
                }

                default:
                    if (is_digit(c)) {
                        // integer
                    }
                    else if (is_valid_symbol_char(c)) {
                    }
                    break;
                }
            }

            observer.close_file();
        }

        void rewind() {
            assert(memcmp(&curr_state_, &prev_state_, sizeof(curr_state_)) != 0);
            curr_state_ = prev_state_;
        }

        void reset(const char* src) {
            reset(src, strlen(src));
        }

        void reset(const char* src, size_t len) {
            assert(src);

            src_ = src;
            curr_state_ = state{};
            prev_state_ = state{};

            std::string tmpstr;
        }

    private:
        const char& getc() {
            const char& c = src_[curr_state_.off];
            if (c != '\0') {
                ++curr_state_.off;
            }
        }

        void ungetc() {
            if (curr_state_.off > 0) {
                --curr_state_.off;
            }
        }

    private:
        struct state {
            size_t off = 0;
        };

        const char* src_;
        state       curr_state_;
        state       prev_state_;
    };

}
