add_library (eth_mdio_bsp STATIC
    h723_board.cc
)

target_include_directories(eth_mdio_bsp PUBLIC
    .
    ../
)

target_link_libraries(eth_mdio_bsp PUBLIC
    driver
    core
)

