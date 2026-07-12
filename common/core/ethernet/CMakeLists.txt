add_library(ethernet INTERFACE)

add_subdirectory(sma)

target_link_libraries(ethernet INTERFACE
    sma
)