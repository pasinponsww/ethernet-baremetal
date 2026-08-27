add_library (eth_bsp STATIC
    h723_board.cc
)

target_include_directories(eth_bsp PUBLIC
    .
    ../
)

target_link_libraries(eth_bsp PUBLIC
    driver
    core
    phy
    ethernet
)
