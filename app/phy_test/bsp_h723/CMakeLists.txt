add_library (phy_bsp STATIC
    h723_board.cc
)

target_include_directories(phy_bsp PUBLIC
    .
    ../
)

target_link_libraries(phy_bsp PUBLIC
    driver
    core
    phy
)

