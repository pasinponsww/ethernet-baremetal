/**
 * @file udp_server.h
 * @brief Generic UDP server on the lwIP raw API, framed by a PacketProtocol
 * @note One datagram is one frame, so there is no reassembly, no
 *       connection state and no idle timeout; a lost request or response
 *       is the client's problem to time out and retry. TProtocol::frame_size
 *       is still consulted so a truncated or foreign datagram is dropped
 *       rather than handed to the protocol.
 *
 *       Resources, fixed at compile time: two TProtocol::kMaxFrameSize
 *       buffers (receive flatten, response staging). No heap of our own;
 *       each response borrows one PBUF_RAM pbuf from lwIP's MEM_SIZE pool
 *       for the duration of udp_sendto.
 *
 *       Ownership: holds the protocol by reference (must outlive this).
 *       Owns the udp pcb from start() to stop(). lwIP owns the pbuf it
 *       passes to on_recv, which frees it on every path.
 *
 * @author Bex Saw
 */

#pragma once
#include <array>
#include <cstdint>
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "packet_protocol.h"

namespace EoT::Net
{

/**
 * @brief Datagram-level counters
 * @param received Datagrams delivered to this server
 * @param requests Frames handed to the protocol
 * @param dropped Datagrams discarded: oversize, truncated or Invalid
 * @param send_failures Responses that could not be queued
 */
struct UdpStats
{
    uint32_t received{0U};
    uint32_t requests{0U};
    uint32_t dropped{0U};
    uint32_t send_failures{0U};
};

template <typename TProtocol>
class UdpServer
{
public:
    /**
     * @brief What the BSP chooses about this socket
     * @param port UDP port to bind
     */
    struct Config
    {
        uint16_t port{TProtocol::kDefaultPort};
    };

    explicit UdpServer(PacketProtocol<TProtocol>& protocol) : protocol(protocol)
    {
    }

    /**
     * @brief Bind the UDP port and start receiving
     * @return true if the port was bound
     * @note Call after NetIf::init(). Works with the link down.
     */
    bool start(const Config& config = {});

    /**
     * @brief Unbind the port. Idempotent
     */
    void stop();

    bool is_bound() const
    {
        return pcb != nullptr;
    }

    const UdpStats& statistics() const
    {
        return stats;
    }

private:
    /**
     * @brief lwIP receive callback
     * @note Static because lwIP takes a C function pointer.
     */
    static void on_recv(void* arg, udp_pcb* upcb, pbuf* p,
                        const ip_addr_t* addr, u16_t port);

    PacketProtocol<TProtocol>& protocol;
    Config config{};
    udp_pcb* pcb{nullptr};
    UdpStats stats{};

    // The datagram may arrive as a pbuf chain; flattened here so the
    // protocol sees one contiguous frame.
    std::array<uint8_t, TProtocol::kMaxFrameSize> rx_buffer{};

    // Response staging, kept off the callback stack.
    std::array<uint8_t, TProtocol::kMaxFrameSize> tx_buffer{};
};

}  // namespace EoT::Net

#include "udp_server_impl.h"
