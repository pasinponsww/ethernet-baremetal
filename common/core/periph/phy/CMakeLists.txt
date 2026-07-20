add_library(phy INTERFACE)

target_include_directories(phy
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../../drivers/io
)

target_link_libraries(phy
    INTERFACE
        driver
)