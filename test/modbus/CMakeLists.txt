# packet/modbus never includes lwIP, so it links and runs on the host as-is.
# The generic transports (TcpServer<T>/UdpServer<T>) are not covered here:
# they need a live lwIP stack behind them.
add_tests(modbus
    mbap_test
    data_test
    protocol_test
)
