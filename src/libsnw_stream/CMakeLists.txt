set(SNW_SRCS
    stream_buffer.cpp
)

set(SNW_HDRS
    snw_stream.h
    stream_buffer.h
    byte_stream.h
    byte_stream.hpp
    message_stream.h
    message_stream.hpp
)

set(SNW_LIBS
    snw_util
)

add_library(snw_stream ${SNW_SRCS} ${SNW_HDRS})
target_include_directories(snw_stream PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(snw_stream ${SNW_LIBS})
