add_library (eth_dma_bsp STATIC
    h723_board.cc
)

target_include_directories(eth_dma_bsp PUBLIC
    .
    ../
)

target_link_libraries(eth_dma_bsp PUBLIC
    driver
    core
)
