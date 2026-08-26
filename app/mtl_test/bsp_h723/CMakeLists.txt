add_library (mtl_bsp STATIC
    h723_board.cc
)

target_include_directories(mtl_bsp PUBLIC
    .
    ../
)

target_link_libraries(mtl_bsp PUBLIC
    driver
    core
)
