#pragma once

#include <type_traits>

namespace snw {
namespace detail {

template<int index, typename Target, typename... Ts>
struct find_type_util;

template<int index, typename Target, typename T, typename... Ts>
struct find_type_util<index, Target, T, Ts...> {
    enum {
        value = std::is_same<Target, T>::value ? index : find_type_util<index+1, Target, Ts...>::value
    };
};

template<int index, typename Target>
struct find_type_util<index, Target> {
    enum {
        value = -1
    };
};

}

template<typename Target, typename... Ts>
struct find_type {
    enum {
        value = detail::find_type_util<0, Target, Ts...>::value
    };
};

}
