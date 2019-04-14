set(SNW_SRCS
    main.cpp
    t_util_box.cpp
    t_util_bits.cpp
    t_util_align.cpp
    t_util_array.cpp
    t_util_intrusive_list.cpp
    t_util_dfa16_state_machine.cpp
    t_util_find_type.cpp
    t_util_function.cpp
    t_util_varchar.cpp
    t_stream_stream_buffer.cpp
    t_event_future.cpp
    t_mem_page_list.cpp
    t_mem_page_stack.cpp
    t_lang_text_reader.cpp
    t_lang_lexer.cpp
)

set(SNW_HDRS
    catch.hpp
)

set(SNW_LIBS
    snw_util
    snw_stream
    snw_event
    snw_mem
    snw_lang
)

add_executable(unit_test ${SNW_SRCS} ${SNW_HDRS})

target_link_libraries(unit_test LINK_PUBLIC ${SNW_LIBS} rt)
