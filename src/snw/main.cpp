#include <iostream>

#include "snw_util.h"
#include "snw_core.h"

struct allocation_event {
    void* addr;
    size_t size;
};

struct deallocation_event {
    void* addr;
    size_t size;
};

template<typename Core>
class memory_module : public snw::module<Core, memory_module> {
    using module = snw::module<Core, memory_module>;
public:
    memory_module(Core& core)
        : module(core)
    {
    }

    void* allocate(size_t size) {
        void* addr = ::malloc(size);
        this->emit_event(allocation_event{
            addr,
            size
        });
        return addr;
    }

    void deallocate(void* addr, size_t size) {
        this->emit_event(deallocation_event{
            addr,
            size
        });
        ::free(addr);
    }
};

template<typename Core>
class memory_audit_module : public snw::module<Core, memory_audit_module> {
    using module = snw::module<Core, memory_audit_module>;
public:
    memory_audit_module(Core& core)
        : module(core)
    {
    }

    void handle_event(const allocation_event& event) {
        (void)event;
        std::cout << "allocate" << std::endl;
    }

    void handle_event(const deallocation_event& event) {
        (void)event;
        std::cout << "free" << std::endl;
    }
};

namespace snw {
    template<>
    struct subscription_list<memory_audit_module> {
        using events = event_list<
            allocation_event,
            deallocation_event
        >;
    };
}

int main(int argc, char** argv) {
    using my_core = snw::core<
        memory_module,
        memory_audit_module
    >;

    my_core core;
    memory_module<my_core> memory(core);
    memory_audit_module<my_core> memory_audit(core);

    // std::cout << memory.is_subscribed<allocation_event>() << std::endl;

    {
        auto chunk = memory.allocate(16);
        memory.deallocate(chunk, 16);
    }

#ifdef SNW_OS_WINDOWS
    std::system("pause");
#endif
    return 0;
}
