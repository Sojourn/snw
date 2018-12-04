set(SNW_SRCS
    snw_event.cpp
)

set(SNW_HDRS
    snw_event.h
    event_router.h
    event_dispatcher.h
    subscription_list.h
    future.h
    future.hpp
)

set(SNW_LIBS
    snw_util
)

add_library(snw_event ${SNW_SRCS} ${SNW_HDRS})
target_include_directories(snw_event PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(snw_event ${SNW_LIBS})
