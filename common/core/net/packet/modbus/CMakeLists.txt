# Modbus TCP/IP: vocabulary, MBAP codec, PDU dispatcher and the
# PacketProtocol that carries them over TcpServer<T> / UdpServer<T>.
add_library(modbus STATIC
    modbus.cc
    mbap.cc
    data.cc
    protocol.cc
)

# Includes are rooted at packet/ ("modbus/data.h"), which packet already
# exports, so nothing extra to add here.
#
# utils: modbus.h's read_be16 forwards to reg_helpers.h's combine_uint16,
# the same pattern lwip already uses to get delay.h.
target_link_libraries(modbus PUBLIC
    packet
    utils
)
