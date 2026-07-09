add_library (mdio_bsp STATIC
    h723_board.cc
)

target_include_directories(mdio_bsp PUBLIC
    .
    ../
)

target_link_libraries(mdio_bsp PUBLIC
    driver
    core
)

