set(SNW_SRCS
    snw_util.cpp
    platform.cpp
    dfa16_state_machine.cpp
    slot_allocator.cpp
)

set(SNW_HDRS
    snw_util.h
    align.h
    platform.h
    bits.h
    box.h
    box.hpp
    array.h
    hash.h
    intrusive_list.h
    intrusive_list.hpp
    dfa16_state_machine.h
    find_type.h
    function.h
    function.hpp
    varchar.h
    varchar.hpp
    type_traits.h
    slot_allocator.h
    slot_allocator.hpp
    registry.h
)

add_library(snw_util ${SNW_SRCS} ${SNW_HDRS})
target_include_directories(snw_util PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
