set(SNW_SRCS
    lexer.cpp
    object_handle.cpp
    object_heap.cpp
)

set(SNW_HDRS
    snw_lang.h
    text_reader.h
    lexer.h
    parser.h
    object.h
    object_handle.h
    object_heap.h
)

set(SNW_LIBS
    snw_util
)

add_library(snw_lang ${SNW_SRCS} ${SNW_HDRS})
target_include_directories(snw_lang PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(snw_lang ${SNW_LIBS})
