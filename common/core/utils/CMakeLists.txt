add_library(utils STATIC
    reg_helpers.cc
    delay.cc
)

target_include_directories(utils PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

