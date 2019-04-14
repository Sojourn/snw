set(SNW_SRCS
    main.cpp
)

set(SNW_HDRS
)

set(SNW_LIBS
    snw_util
    snw_stream
    snw_event
    snw_mem
    snw_lang
)

add_executable(snw ${SNW_SRCS} ${SNW_HDRS})

target_link_libraries(snw LINK_PUBLIC ${SNW_LIBS} rt)
