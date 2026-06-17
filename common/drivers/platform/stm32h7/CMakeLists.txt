
add_library(hal STATIC
    st_gpio.cc
)

target_include_directories(hal PUBLIC
    .
    ${CMAKE_SOURCE_DIR}/common/core/utils
)

Uncomment this when you adding the st_gpio.cc/.h
target_link_libraries(hal PUBLIC
    st_h7_support
    core 
)