#pragma once

#include <vector>
#include <cstddef>
#include "object.h"
#include "lexer.h"
#include "array.h"

namespace snw {

class object_heap;

class parser {
    friend class lexer;
public:
    parser(object_heap& heap);

    object_ref parse(const char* str);

private:
    void open_file();
    void close_file();
    void open_list();
    void close_list();
    void integer(int64_t value);
    void string(const char* first, const char* last);
    void symbol(const char* first, const char* last);
    void comment(const char* first, const char* last);
    void error(const snw::lexer_error& err);

private:
    object_ref pop_list();

private:
#if 0
    static constexpr size_t max_frame_size_ = 128;
    static constexpr size_t max_stack_size_ = 128;

    using frame = array<object_ref, max_frame_size_>;
    using stack = array<frame, max_stack_size_>;
#else
    using frame = std::vector<object_ref>;
    using stack = std::vector<frame>;
#endif

    object_heap& heap_;
    object_ref   program_;
    stack        stack_;
};

}
