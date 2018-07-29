#pragma once

#include "find_type.h"

namespace snw {

template<typename... Events>
struct event_list{};

template<template<typename> class Module, typename... Events>
struct subscription_list;

template<template<typename> class Module>
struct subscription_list<Module> {
    using events = event_list<>;
};

namespace detail {
    template<typename Event, typename... Events>
    constexpr int find_event(event_list<Events...>) {
        return find_type<Event, Events...>::value;
    }

    template<template<typename> class Module, typename Event>
    constexpr bool is_subscribed() {
        using subscriptions = subscription_list<Module>;
        using events = typename subscriptions::events;
        return find_event<Event>(events{}) >= 0;
    }
}

}
