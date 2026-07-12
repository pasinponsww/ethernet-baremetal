add_library(phy
    lan8742.cc
)

target_include_directories(phy
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(phy
    PUBLIC
        io
)