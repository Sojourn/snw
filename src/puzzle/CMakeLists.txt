set(SNW_SRCS
    botw_akh_vaquot_shrine.cpp
)

set(SNW_HDRS
)

set(SNW_LIBS
)

add_executable(puzzle ${SNW_SRCS} ${SNW_HDRS})

target_link_libraries(puzzle LINK_PUBLIC ${SNW_LIBS})
