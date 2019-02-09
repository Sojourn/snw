#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <bitset>
#include <memory>
#include <cstring>
#include <cstdint>
#include <cstddef>

#include "snw_util.h"
#include "snw_event.h"
#include "snw_lang.h"

struct dummy_parser {
    void open_file() {
        std::cout << "open_file" << std::endl;
    }
    void open_list() {
        std::cout << "open_list" << std::endl;
    }
    void integer(int64_t value) {
        std::cout << "integer:" << value << std::endl;
    }
    void string(const char* first, const char* last) {
        std::cout << "string:" << std::string(first, last-first) << std::endl;
    }
    void symbol(const char* first, const char* last) {
        std::cout << "symbol:" << std::string(first, last-first) << std::endl;
    }
    void comment(const char* first, const char* last) {
        std::cout << "comment:" << std::string(first, last-first) << std::endl;
    }
    void close_list() {
        std::cout << "close_list" << std::endl;
    }
    void close_file() {
        std::cout << "close_file" << std::endl;
    }
    void error(const snw::lexer_error& err) {
        std::cout << "error(msg:'" << err.msg << "' off:" << err.off << " column:" << err.column << " row:" << err.row << ")" << std::endl;
    }
};

int main(int argc, char** argv) {

#ifdef SNW_OS_WINDOWS
    std::system("pause");
#endif
    return 0;
}
