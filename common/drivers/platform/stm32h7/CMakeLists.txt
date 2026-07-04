
add_library(hal STATIC
    st_gpio.cc
    st_sysclk.cc
    st_usart.cc
    st_crc.cc
    st_rcc.cc
    st_mdio.cc
)

target_include_directories(hal PUBLIC
    .
    ${CMAKE_SOURCE_DIR}/mcu_support/stm32/h7xx/
    ${CMAKE_SOURCE_DIR}/common/drivers/io
    ${CMAKE_SOURCE_DIR}/common/drivers/checksum
    ${CMAKE_SOURCE_DIR}/common/core/utils
)

#Uncomment this when you adding the st_gpio.cc/.h
target_link_libraries(hal PUBLIC
    st_h7_support
    core 
)