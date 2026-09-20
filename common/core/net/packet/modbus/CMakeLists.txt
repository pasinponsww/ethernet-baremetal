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
target_link_libraries(modbus PUBLIC
    packet
)
