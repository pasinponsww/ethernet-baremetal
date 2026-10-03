/**
 * @file transport_test.cc
 * @brief Instantiates TcpServer<T> / UdpServer<T> over Modbus::Protocol and
 *        checks their idle state. No lwIP is started
 *
 * @author Bex Saw
 */

#include <cstdint>
#include <type_traits>

#include <gtest/gtest.h>

#include "modbus/data.h"
#include "modbus/protocol.h"
#include "tcp_server.h"
#include "udp_server.h"

// Explicit instantiation compiles every member, not just the ones called.
template class EoT::Net::TcpServer<EoT::Net::Modbus::Protocol>;
template class EoT::Net::UdpServer<EoT::Net::Modbus::Protocol>;

namespace
{

using namespace EoT::Net;
using namespace EoT::Net::Modbus;

TEST(Transport, ConfigDefaultsComeFromProtocol)
{
    EXPECT_EQ(TcpServer<Protocol>::Config{}.port, Spec::kDefaultPort);
    EXPECT_EQ(UdpServer<Protocol>::Config{}.port, Spec::kDefaultPort);
}

TEST(Transport, ConstructedServersAreIdle)
{
    DataModel model{};
    Server server{model};
    Protocol protocol{server};

    TcpServer<Protocol> tcp{protocol};
    UdpServer<Protocol> udp{protocol};

    EXPECT_FALSE(tcp.is_listening());
    EXPECT_FALSE(udp.is_bound());
    EXPECT_EQ(tcp.statistics().accepted, 0U);
    EXPECT_EQ(udp.statistics().received, 0U);

    // stop() on a never-started server is a no-op, not a crash.
    tcp.stop();
    udp.stop();
}

TEST(Transport, MaxConnectionsIsATemplateParameter)
{
    DataModel model{};
    Server server{model};
    Protocol protocol{server};

    TcpServer<Protocol, 2U> two{protocol};
    TcpServer<Protocol, 8U> eight{protocol};

    // Each slot carries one kMaxFrameSize reassembly buffer.
    static_assert(sizeof(eight) > sizeof(two) + 5U * Protocol::kMaxFrameSize);
    EXPECT_FALSE(two.is_listening());
    EXPECT_FALSE(eight.is_listening());
}

}  // namespace
