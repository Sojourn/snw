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

template<typename core>
class memory_module {
public:
    memory_module(core& c)
        : core_(c)
    {
    }

    ~memory_module() {
        core_.template unregister_module<memory_module>();
    }

    void* allocate(size_t size) {
        void* addr = ::malloc(size);
        emit_event(allocation_event{
            addr,
            size
        });
        return addr;
    }

    void deallocate(void* addr, size_t size) {
        emit_event(deallocation_event{
            addr,
            size
        });
        ::free(addr);
    }

private:
    template<typename Event>
    void emit_event(const Event& event) {
        core_.template emit_event(event);
    }

private:
    core& core_;
};

template<typename core>
class memory_audit_module {
public:
    memory_audit_module(core& cr)
        : core_(cr)
    {
        core_.register_module(this);
    }

    ~memory_audit_module() {
        core_.template unregister_module<memory_audit_module>();
    }

    void handle_event(const allocation_event& event) {
        (void)event;
        std::cout << "allocate" << std::endl;
    }

    void handle_event(const deallocation_event& event) {
        (void)event;
        std::cout << "free" << std::endl;
    }

private:
    core& core_;
};

namespace snw {
    template<>
    struct subscription_list<
        memory_audit_module,
        allocation_event,
        deallocation_event
    > {};
}

int main(int argc, char** argv) {
    using core = snw::core<
        memory_module,
        memory_audit_module
    >;

    core cr;
    memory_module<core> memory(cr);
    memory_audit_module<core> memory_audit(cr);

    {
        auto chunk = memory.allocate(16);
        memory.deallocate(chunk, 16);
    }

#ifdef SNW_OS_WINDOWS
    std::system("pause");
#endif
    return 0;
}
