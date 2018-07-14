if(WIN32)
    # TODO
else()
    add_compile_options(-std=c++11)
    add_compile_options(-msse3)
endif()
