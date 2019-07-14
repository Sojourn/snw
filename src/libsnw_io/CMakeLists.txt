set(SNW_SRCS
    address.cpp
    socket.cpp
)

set(SNW_HDRS
    snw_io.h
    address.h
    socket.h
)

set(SNW_LIBS
    snw_util
    snw_stream
)

add_library(snw_io ${SNW_SRCS} ${SNW_HDRS})
target_include_directories(snw_io PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(snw_io ${SNW_LIBS})
