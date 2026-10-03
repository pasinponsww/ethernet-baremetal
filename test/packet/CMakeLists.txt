# The generic transports are templates, so nothing compiles them until they
# are instantiated. This test instantiates both over Modbus::Protocol so a
# signature drift between the PacketProtocol contract and a transport fails
# here rather than in an application build. lwIP is linked but never
# started: constructing a server touches no lwIP state.
add_tests("modbus;lwip"
    transport_test
)
