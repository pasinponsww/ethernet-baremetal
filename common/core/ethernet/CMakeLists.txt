add_library(ethernet INTERFACE)

target_include_directories(ethernet INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ethernet INTERFACE
)