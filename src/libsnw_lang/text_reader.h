#pragma once

#include <algorithm>
#include <cstring>
#include <cstddef>
#include <cassert>

namespace snw {

template<size_t max_history>
class text_reader {
    static constexpr size_t max_history_ = max_history + 1;
public:
    text_reader(const char* str)
        : str_(str)
        , off_(0)
        , history_(0)
    {
        memset(history_entries_, 0, sizeof(history_entries_));
    }

    const char& getc() {
        const char& c = str_[off_];
        if (c != '\0') {
            ++off_;

            // this history entry might have replaced a previous one
            history_ = std::min(history_ + 1, max_history);
        }

        auto& hent = get_history_entry(0);

        hent = get_history_entry(1);
        hent.update_column(c);
        hent.update_row(c);

        return c;
    }

    void ungetc() {
        assert(history_ > 0);
        --off_;
        --history_;
    }

    size_t offset() const {
        assert(off_ > 0);
        return off_ - 1;
    }

    size_t column() const {
        assert(off_ > 0);
        return get_history_entry(1).column;
    }

    size_t row() const {
        assert(off_ > 0);
        return get_history_entry(1).row;
    }

private:
    struct history_entry {
        size_t column;
        size_t row;

        history_entry()
            : column(0)
            , row(0)
        {
        }

        void update_column(char c) {
            if ((c == '\r') || (c == '\n')) {
                column = 0;
            }
            else if (c == '\t') {
                column += 4;
            }
            else {
                ++column;
            }
        }

        void update_row(char c) {
            if (c == '\n') {
                ++row;
            }
        }
    };

    history_entry& get_history_entry(size_t age) {
        return history_entries_[history_ - age];
    }

    const history_entry& get_history_entry(size_t age) const {
        return history_entries_[history_ - age];
    }

private:
    const char*   str_;
    size_t        off_;
    size_t        history_;
    history_entry history_entries_[max_history];
};

}