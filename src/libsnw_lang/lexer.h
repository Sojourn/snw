#pragma once

#include <stdexcept>
#include <string>
#include <cstring>
#include <cstddef>
#include <cstdint>
#include <cassert>
#include "text_reader.h"

namespace snw {

    inline bool is_space(char c) {
        switch (c) {
        case ' ':
        case '\t':
        case '\r':
        case '\n':
            return true;

        default:
            return false;
        }
    }

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

    struct lexer_error {
        const char* msg;
        size_t      off;
        size_t      column;
        size_t      row;
    };

    // struct lexer_observer {
    //     void open_file();
    //     void open_list();
    //     void integer(int64_t);
    //     void string(const char* first, const char* last);
    //     void symbol(const char* first, const char* last);
    //     void close_list();
    //     void close_file();
    //     void error(const lexer_error& err);
    // };

    class lexer {
    public:
        lexer(const char* src)
            : reader_(src)
            , state_(state::initial)
        {
        }

        template<typename Parser>
        bool next(Parser& parser) {
            switch (state_) {
            case state::initial:
                return process_initial_state(parser);

            case state::running:
                return process_running_state(parser);

            default:
            case state::finished:
                return process_finished_state(parser);
            }
        }

    private:
        lexer_error make_lexer_error(const char* msg) const {
            lexer_error err;
            err.msg = msg;
            err.off = reader_.offset();
            err.column = reader_.column();
            err.row = reader_.row();
            return err;
        }

        template<typename Parser>
        bool process_initial_state(Parser& parser) {
            state_ = state::running;
            parser.open_file();
            return true;
        }

        template<typename Parser>
        bool process_running_state(Parser& parser) {
            while (const char& c = reader_.getc()) {
                switch (c) {
                // whitespace
                case ' ':
                case '\t':
                case '\r':
                case '\n':
                    continue;

                // line comment
                case ';': {
                    const char* first = &c + 1;
                    const char* last = nullptr;

                    do {
                        last = &reader_.getc();
                    } while (*last && (*last != '\n'));

                    parser.comment(first, last);
                    return true;
                }

                case '(':
                    parser.open_list();
                    return true;

                case ')':
                    parser.close_list();
                    return true;

                case '"': {
                    const char* first = (&c) + 1;
                    const char* last = nullptr;
                    bool escaped = false;

                    while (const char& c = reader_.getc()) {
                        if (!is_valid_string_char(c)) {
                            state_ = state::finished;
                            parser.error(make_lexer_error("invalid string char"));
                            return false;
                        }

                        // TODO: unescape and copy
                        if (escaped) {
                            if (is_escapable_char(c)) {
                                escaped =  false;
                            }
                            else {
                                state_ = state::finished;
                                parser.error(make_lexer_error("invalid escape"));
                                return false;
                            }
                        }
                        else if (c == '"') {
                            last = &c;
                            parser.string(first, last);
                            return true;
                        }
                    }

                    state_ = state::finished;
                    parser.error(make_lexer_error("untermianted string"));
                    return false;
                }

                default:
                    if (is_digit(c)) {
                        int64_t value = (c - '0');
                        while (const char& c = reader_.getc()) {
                            if (is_digit(c)) {
                                // TODO: overflow check
                                value *= 10;
                                value += (c - '0');
                            }
                            else {
                                break;
                            }
                        }

                        parser.integer(value);
                        return true;
                    }
                    else if (is_valid_symbol_char(c)) {
                        const char* first = &c;
                        const char* last = nullptr;

                        do {
                            last = &reader_.getc();
                        } while (*last && is_valid_symbol_char(*last));

                        reader_.ungetc();
                        parser.symbol(first, last);
                        return true;
                    }
                    else {
                        state_ = state::finished;
                        parser.error(make_lexer_error("invalid token character"));
                        return false;
                    }
                }
            }

            state_ = state::finished;
            parser.close_file();
            return true;
        }

        template<typename Parser>
        bool process_finished_state(Parser& parser) {
            return false;
        }

    private:
        enum class state {
            initial,
            running,
            finished,
        };

        text_reader<2> reader_;
        state          state_;
    };

    template<typename Parser>
    void parse(const char* source, Parser& parser) {
        lexer l(source);
        while (l.next(parser)) {
            // pass
        }
    }
}
