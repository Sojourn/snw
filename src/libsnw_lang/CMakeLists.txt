set(SNW_SRCS
    object_ref.cpp
    lexer.cpp
    parser.cpp
)

set(SNW_HDRS
    snw_lang.h
    object_ref.h
    object.h
    object_heap.h
    object_stack.h
    object_transaction.h
    text_reader.h
    lexer.h
    parser.h
)

set(SNW_LIBS
    snw_util
)

add_library(snw_lang ${SNW_SRCS} ${SNW_HDRS})
target_include_directories(snw_lang PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(snw_lang ${SNW_LIBS})
