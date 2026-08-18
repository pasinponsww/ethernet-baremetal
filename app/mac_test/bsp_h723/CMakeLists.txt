add_library (mac_bsp STATIC
    h723_board.cc
)

target_include_directories(mac_bsp PUBLIC
    .
    ../
)

target_link_libraries(mac_bsp PUBLIC
    driver
    core
)
