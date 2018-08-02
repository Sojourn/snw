#include <iostream>
#include <string>
#include <vector>

#include "snw_util.h"
#include "snw_core.h"

struct status_event_request {
};

enum class status_event_severity {
    warning,
    error
};

struct status_event {
    status_event_severity severity;
    std::string           msg;
};

template<typename Core>
class status_event_collector {
public:
    status_event_collector(Core& core)
        : core_(core)
    {
        core_.register_module(this);
    }

    ~status_event_collector()
    {
        core_.unregister_module(this);
    }

    std::vector<status_event> query_status_events() {
        try {
            core_.emit_event(status_event_request{});
        }
        catch(const std::exception& ex) {
            status_events_.push_back(status_event{
                status_event_severity::error,
                ex.what()
            });
        }

        return std::move(status_events_);
    }

    void handle_event(const status_event& evt) {
        status_events_.push_back(evt);
    }

private:
    Core& core_;
    std::vector<status_event> status_events_;
};

namespace snw {
    template<>
    struct subscription_list<status_event_collector> {
        using events = event_list<
            status_event
        >;
    };
}

template<typename Core>
class thing_a {
public:
    thing_a(Core& core)
        : core_(core)
    {
        core_.register_module(this);
    }

    ~thing_a()
    {
        core_.unregister_module(this);
    }

    void handle_event(const status_event_request&) {
        core_.emit_event(status_event{
            status_event_severity::error,
            "An error"
        });
    }

private:
    Core& core_;
};

namespace snw {
    template<>
    struct subscription_list<thing_a> {
        using events = event_list<
            status_event_request
        >;
    };
}

template<typename Core>
class thing_b {
public:
    thing_b(Core& core)
        : core_(core)
    {
        core_.register_module(this);
    }

    ~thing_b()
    {
        core_.unregister_module(this);
    }

    void handle_event(const status_event_request&) {
        core_.emit_event(status_event{
            status_event_severity::warning,
            "A warning"
        });
    }

private:
    Core& core_;
};

namespace snw {
    template<>
    struct subscription_list<thing_b> {
        using events = event_list<
            status_event_request
        >;
    };
}

int main(int argc, char** argv) {
    using my_core = snw::core<
        status_event_collector,
        thing_a,
        thing_b
    >;

    my_core core;
    status_event_collector<my_core> sec{core};
    thing_a<my_core> ta{core};
    thing_b<my_core> tb{core};

    for(const status_event& se: sec.query_status_events()) {
        std::cout << se.msg << std::endl;
    }

#ifdef SNW_OS_WINDOWS
    std::system("pause");
#endif
    return 0;
}
