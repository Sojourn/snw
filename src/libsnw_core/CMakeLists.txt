set(SNW_SRCS
    snw_core.cpp
)

set(SNW_HDRS
    snw_core.h
    core.h
    subscription_list.h
    future.h
    future.hpp
)

set(SNW_LIBS
    snw_util
)

add_library(snw_core ${SNW_SRCS} ${SNW_HDRS})
target_include_directories(snw_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(snw_core ${SNW_LIBS})
