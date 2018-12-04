#pragma once

#include <tuple>
#include <cassert>
#include "find_type.h"
#include "subscription_list.h"
#include "event_dispatcher.h"

namespace snw {

template<template<typename> class... Modules>
class event_router {
    template<template<typename> class Module>
    static constexpr int find_module() {
        return find_type<Module<event_router<Modules...>>, Modules<event_router<Modules...>>...>::value;
    }

    template<typename Module>
    static constexpr int find_qualified_module() {
        return find_type<Module, Modules<event_router<Modules...>>...>::value;
    }

public:
    template<typename Module>
    void register_module(Module* module) {
        enum {
            index = find_qualified_module<Module>()
        };
        static_assert(index >= 0, "Add module to event_router");
        assert(module);
        std::get<index>(modules_) = module;
    }

    template<typename Module>
    void unregister_module(Module*) {
        enum {
            index = find_qualified_module<Module>()
        };
        static_assert(index >= 0, "Add module to event_router");
        std::get<index>(modules_) = nullptr;
    }

    template<template<typename> class Module>
    Module<event_router<Modules...>>& get() {
        enum {
            index = find_module<Module>()
        };
        static_assert(index >= 0, "Add module to event_router");
        return *std::get<index>(modules_);
    }

    template<template<typename> class Module>
    const Module<event_router<Modules...>>& get() const {
        enum {
            index = find_module<Module>()
        };
        static_assert(index >= 0, "Add module to event_router");
        return *std::get<index>(modules_);
    }

    // Send an event to subscribed modules
    template<typename Event>
    void send(const Event& event) {
        dispatch_event<Modules...> dispatcher;
        dispatcher(modules_, event);
    }

private:
    std::tuple<Modules<event_router<Modules...>>*...> modules_;
};

}
