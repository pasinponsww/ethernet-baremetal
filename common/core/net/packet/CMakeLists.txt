# Header-only: packet_protocol.h is the CRTP facade (EoT::Net::PacketProtocol<T>)
# and tcp_server.h / udp_server.h are the generic lwIP transports templated on
# it. Concrete protocols live in their own subdirectory and link this.
add_library(packet INTERFACE)

target_include_directories(packet INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(packet INTERFACE
    lwip
)

add_subdirectory(modbus)
