#pragma once

#include <tuple>
#include "subscription_list.h"

namespace snw {
namespace detail {

// TODO: replace with 'if constexpr'
template<bool subscribed>
struct notify_event;

template<>
struct notify_event<true> {
    template<typename Module, typename Event>
    void operator()(Module& module, const Event& event) {
        module.handle_event(event);
    }
};

template<>
struct notify_event<false> {
    template<typename Module, typename Event>
    void operator()(Module&, const Event&) {
    }
};

template<size_t index, template<typename> class... Modules>
struct dispatch_event_util;

template<size_t index, template<typename> class Module, template<typename> class... Modules>
struct dispatch_event_util<index, Module, Modules...> {
    template<typename ModuleTuple, typename Event>
    void operator()(ModuleTuple& modules, const Event& event) {
        auto module = std::get<index>(modules);
        assert(module && "is module registered?");

        notify_event<is_subscribed<Module, Event>()> notifier;
        notifier(*module, event);

        dispatch_event_util<index+1, Modules...> dispatcher;
        dispatcher(modules, event);
    }
};

template<size_t index>
struct dispatch_event_util<index> {
    template<typename ModuleTuple, typename Event>
    void operator()(ModuleTuple&, const Event&) {
        // base case
    }
};

}

template<template<typename> class... Modules>
struct dispatch_event {
    template<typename ModuleTuple, typename Event>
    void operator()(ModuleTuple& modules, const Event& event) {
        detail::dispatch_event_util<0, Modules...> dispatcher;
        dispatcher(modules, event);
    }
};

}
