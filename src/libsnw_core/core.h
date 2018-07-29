#pragma once

#include <tuple>
#include "intrusive_list.h"
#include "find_type.h"

namespace snw {

template<template<typename> class Module, typename... Events>
struct subscription_list;

template<template<typename> class Module>
struct subscription_list<Module> {
};

// template<typename ModuleTuple, typename Event, int index=0>
// struct EventDispatcher {
//     void operator()(ModuleTuple& modules, const Event& event) {
//     }
// };

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

    template<template<typename> class Module>
    void unregister_module() {
        enum {
            index = find_module<Module>()
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

    template<typename Event>
    void emit_event(const Event& event) {
        // TODO
    }

private:
    std::tuple<Modules<core<Modules...>>*...> modules_;
};

}
