#pragma once

#include <tuple>
#include <cassert>
#include "find_type.h"
#include "subscription_list.h"
#include "event_dispatcher.h"

namespace snw {

template<template<typename> class... Modules>
class core {
    template<template<typename> class Module>
    static constexpr int find_module() {
        return find_type<Module<core<Modules...>>, Modules<core<Modules...>>...>::value;
    }

    template<typename Module>
    static constexpr int find_qualified_module() {
        return find_type<Module, Modules<core<Modules...>>...>::value;
    }

public:
    template<typename Module>
    void register_module(Module* module) {
        enum {
            index = find_qualified_module<Module>()
        };
        static_assert(index >= 0, "Add module to core");
        assert(module);
        std::get<index>(modules_) = module;
    }

    template<typename Module>
    void unregister_module(Module*) {
        enum {
            index = find_qualified_module<Module>()
        };
        static_assert(index >= 0, "Add module to core");
        std::get<index>(modules_) = nullptr;
    }

    template<template<typename> class Module>
    Module<core<Modules...>>& get() {
        enum {
            index = find_module<Module>()
        };
        static_assert(index >= 0, "Add module to core");
        return *std::get<index>(modules_);
    }

    template<template<typename> class Module>
    const Module<core<Modules...>>& get() const {
        enum {
            index = find_module<Module>()
        };
        static_assert(index >= 0, "Add module to core");
        return *std::get<index>(modules_);
    }

    // Forward an event to subscribed modules
    template<typename Event>
    void emit_event(const Event& event) {
        dispatch_event<Modules...> dispatcher;
        dispatcher(modules_, event);
    }

private:
    std::tuple<Modules<core<Modules...>>*...> modules_;
};

}
