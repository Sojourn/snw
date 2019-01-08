#include <iostream>
#include <string>
#include <vector>

#include "snw_util.h"
#include "snw_event.h"

using varchar16 = snw::varchar16;

int main(int argc, char** argv) {
    varchar16 s = "hello, world";

    // std::cout << s << std::endl;
    // std::cout << (s == varchar16("nah")) << std::endl;
    // std::cout << (s == varchar16("hello, world")) << std::endl;
    std::cout << (varchar16("a") >= varchar16("a")) << std::endl;
    std::cout << (varchar16("a") < varchar16("b")) << std::endl;

#ifdef SNW_OS_WINDOWS
    std::system("pause");
#endif
    return 0;
}
