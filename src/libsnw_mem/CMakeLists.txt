set(SNW_SRCS
    snw_mem.cpp
)

set(SNW_HDRS
    snw_mem.h
    box.h
    box.hpp
)

add_library(snw_mem ${SNW_SRCS} ${SNW_HDRS})
target_include_directories(snw_mem PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
