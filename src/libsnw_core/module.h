#pragma once

#include "subscription_list.h"

namespace snw {

template<typename Core, template<typename> class Impl>
class module {
public:
    module(Core& core)
        : core_(core)
    {
        core_.register_module(self());
    }

    ~module() {
        core_.unregister_module(self());
    }

    module(module&&) = delete;
    module(const module&) = delete;
    module& operator=(module&&) = delete;
    module& operator=(const module&) = delete;

    Core& core() {
        return core_;
    }

    const Core& core() const {
        return core_;
    }

    template<typename Event>
    void emit_event(const Event& event) {
        core_.emit_event(event);
    }

private:
    Impl<Core>* self() {
        return static_cast<Impl<Core>*>(this);
    }

    const Impl<Core>* self() const {
        return static_cast<const Impl<Core>*>(this);
    }

private:
    Core& core_;
};

}
