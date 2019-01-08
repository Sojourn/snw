#include <iostream>
#include <string>
#include <vector>

#include "snw_util.h"
#include "snw_event.h"

#include "array.h"

struct type;

struct member {
    const type*      mb_type;
    snw::varchar<16> mb_name;
};

struct type {
    snw::array<member, 16> typ_members;
};

int main(int argc, char** argv) {
#ifdef SNW_OS_WINDOWS
    std::system("pause");
#endif
    return 0;
}
