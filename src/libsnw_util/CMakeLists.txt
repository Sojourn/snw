set(SNW_SRCS
    snw_util.cpp
)

set(SNW_HDRS
    snw_util.h
    intrusive_list.h
    intrusive_list.hpp
    dfa16_state_machine.h
)

add_library(snw_util ${SNW_SRCS} ${SNW_HDRS})
target_include_directories(snw_util PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
