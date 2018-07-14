set(SNW_SRCS
    main.cpp
    t_util_intrusive_list.cpp
    t_util_box.cpp
)

set(SNW_HDRS
    catch.hpp
)

set(SNW_LIBS
    snw_util
    snw_mem
)

add_executable(unit_test ${SNW_SRCS} ${SNW_HDRS})

target_link_libraries(unit_test LINK_PUBLIC ${SNW_LIBS})
