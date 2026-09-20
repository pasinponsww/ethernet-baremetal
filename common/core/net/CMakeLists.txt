set(LWIP_DIR ${CMAKE_SOURCE_DIR}/external/lightweight-ip)
include(${LWIP_DIR}/src/Filelists.cmake)

# lwipnetif_SRCS also drags in bridgeif/slipif, which this port has no use
# for. Take only the Ethernet input path.
add_library(lwip STATIC
    ${lwipcore_SRCS}
    ${lwipcore4_SRCS}
    ${LWIP_DIR}/src/netif/ethernet.c
    lwip_port/arch/sys_arch.cc
)

target_include_directories(lwip PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/lwip_port
    ${CMAKE_CURRENT_SOURCE_DIR}/lwip_port/arch
    ${LWIP_DIR}/src/include
)

# Vendored third-party C: the root CMakeLists turns on -Werror for our code,
# which we do not get to hold lwIP to. Warnings still print, they just do
# not fail the build.
target_compile_options(lwip PRIVATE -Wno-error)

# sys_arch.cc's sys_now()/sys_jiffies() are backed by EoT::Utils::get_ms_ticks()
target_link_libraries(lwip PUBLIC utils)

# net.h / net_impl.h: the link-state orchestrator and lwIP netif glue.
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

# Application protocols carried over lwIP: the generic TcpServer<T> /
# UdpServer<T> transports and the protocols that plug into them.
add_subdirectory(packet)
