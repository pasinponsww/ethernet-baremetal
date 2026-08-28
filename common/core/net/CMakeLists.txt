set(LWIP_DIR ${CMAKE_SOURCE_DIR}/external/lightweight-ip)
include(${LWIP_DIR}/src/Filelists.cmake)

add_library(lwip STATIC
    ${lwipcore_SRCS}
    ${lwipcore4_SRCS}
    ${lwipnetif_SRCS}
    lwip_port/arch/sys_arch.cc
)

target_include_directories(lwip PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/lwip_port
    ${CMAKE_CURRENT_SOURCE_DIR}/lwip_port/arch
    ${LWIP_DIR}/src/include
)

# sys_arch.cc's sys_now()/sys_jiffies() are backed by EoT::Utils::get_ms_ticks()
target_link_libraries(lwip PUBLIC utils)

add_library(net STATIC
    net.cc
)

target_include_directories(net PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(net PUBLIC
    ethernet
    lwip
)
