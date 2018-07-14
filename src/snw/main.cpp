#include <iostream>

#include "snw_util.h"
#include "intrusive_list.h"

struct handle {
    int value;
    snw::intrusive_list_node il_node;
};

using handle_list = snw::intrusive_list<handle, &handle::il_node>;

int main(int argc, char** argv) {
    handle n1{1};
    handle n2{2};
    handle n3{3};

    handle_list hnd_list;
    hnd_list.push_back(n1);
    hnd_list.push_back(n2);
    hnd_list.push_back(n3);

    for(const handle& hnd: hnd_list) {
        std::cout << hnd.value << std::endl;
    }

    std::system("pause");
    return 0;
}
