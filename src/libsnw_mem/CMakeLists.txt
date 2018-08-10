set(SNW_SRCS
    snw_mem.cpp
    page_list.cpp
    page_stack.cpp
)

set(SNW_HDRS
    snw_mem.h
    page_list.h
    page_stack.h
)

set(SNW_LIBS
    snw_util
    snw_core
)

add_library(snw_mem ${SNW_SRCS} ${SNW_HDRS})
target_include_directories(snw_mem PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(snw_mem ${SNW_LIBS})
