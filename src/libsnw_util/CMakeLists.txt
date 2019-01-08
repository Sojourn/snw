set(SNW_SRCS
    snw_util.cpp
    dfa16_state_machine.cpp
)

set(SNW_HDRS
    snw_util.h
    align.h
    platform.h
    box.h
    box.hpp
    array.h
    intrusive_list.h
    intrusive_list.hpp
    dfa16_state_machine.h
    find_type.h
    function.h
    function.hpp
    varchar.h
    varchar.hpp
)

add_library(snw_util ${SNW_SRCS} ${SNW_HDRS})
target_include_directories(snw_util PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
