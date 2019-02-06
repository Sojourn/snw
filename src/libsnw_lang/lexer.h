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

        template<typename Observer>
        bool next(Observer&& observer) {
            switch (state_) {
            case state::initial:
                state_ = state::file_open;
                observer.open_file();
                return true;

            case state::file_open:
                while (const char& c = reader_.getc()) {
                    switch (c) {
                    // whitespace
                    case ' ':
                    case '\t':
                    case '\r':
                    case '\n':
                        continue;

                    // line comment
                    case ';':
                        while (const char& c = reader_.getc()) {
                            if (c == '\n') {
                                break;
                            }
                        }
                        continue;

                    case '(':
                        observer.open_list();
                        return true;

                    case ')':
                        observer.close_list();
                        return true;

                    case '"': {
                        const char* first = (&c) + 1;
                        const char* last = nullptr;
                        bool escaped = false;

                        while (const char& c = reader_.getc()) {
                            if (!is_valid_string_char(c)) {
                                state_ = state::error;
                                observer.error(make_lexer_error("invalid string char"));
                                return false;
                            }

                            // TODO: unescape and copy
                            if (escaped) {
                                if (is_escapable_char(c)) {
                                    escaped =  false;
                                }
                                else {
                                    state_ = state::error;
                                    observer.error(make_lexer_error("invalid escape"));
                                    return false;
                                }
                            }
                            else if (c == '"') {
                                last = &c;
                                observer.string(first, last);
                                return true;
                            }
                        }

                        state_ = state::error;
                        observer.error(make_lexer_error("untermianted string"));
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

                            observer.integer(value);
                            return true;
                        }
                        else if (is_valid_symbol_char(c)) {
                            // FIXME: this is tricky, make it not tricky
                            const char* first = &c;
                            const char* last = first + 1;

                            while (const char& c = reader_.getc()) {
                                last = &c;
                                if (!is_valid_symbol_char(c)) {
                                    reader_.ungetc();
                                    break;
                                }
                            }

                            observer.symbol(first, last);
                            return true;
                        }
                        else {
                            state_ = state::error;
                            observer.error(make_lexer_error("invalid token character"));
                            return false;
                        }
                    }
                }

                state_ = state::file_closed;
                observer.close_file();
                return true;

            case state::file_closed:
            case state::error:
            default:
                return false;
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

    private:
        enum class state {
            initial,
            file_open,
            file_closed,
            error,
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
