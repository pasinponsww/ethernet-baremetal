/**
 * @file protocol.h
 * @brief Modbus TCP/IP framing as a PacketProtocol
 * @note The piece that plugs Modbus into the generic transports: frames
 *       the byte stream by the MBAP length field, filters on unit id, and
 *       hands the PDU inside to a Server. The same ADU format is used on
 *       TCP and UDP, so one Protocol serves both:
 *
 *         Modbus::Server   modbus{model};
 *         Modbus::Protocol protocol{modbus, {.unit_id = 0x11}};
 *         Net::TcpServer<Modbus::Protocol> tcp{protocol};
 *         Net::UdpServer<Modbus::Protocol> udp{protocol};
 *
 *       Ownership: Protocol holds the Server by reference and the config
 *       by value. The Server (and the arrays behind its DataModel) must
 *       outlive the Protocol; the Protocol must outlive any transport
 *       holding it.
 *
 * @author Bex Saw
 */

#pragma once
#include <cstdint>
#include <span>
#include "modbus/data.h"
#include "modbus/mbap.h"
#include "modbus/modbus.h"
#include "packet_protocol.h"

namespace EoT::Net::Modbus
{

/**
 * @brief What the BSP chooses about this Modbus endpoint
 * @param unit_id Unit id to answer for. Spec::kDefaultUnitId (the
 *        default) answers any unit id; any other value is matched
 *        strictly and mismatches are dropped without a response
 */
struct ProtocolConfig
{
    uint8_t unit_id{Spec::kDefaultUnitId};
};

/**
 * @brief Framing-level counters
 * @param requests ADUs unwrapped and dispatched
 * @param filtered ADUs dropped because the unit id was someone else's
 * @param silent Requests the dispatcher chose not to answer
 */
struct ProtocolStats
{
    uint32_t requests{0U};
    uint32_t filtered{0U};
    uint32_t silent{0U};
};

class Protocol final : public PacketProtocol<Protocol>
{
public:
    static constexpr uint16_t kMaxFrameSize{Spec::kMaxAduSize};
    static constexpr uint16_t kDefaultPort{Spec::kDefaultPort};

    /**
     * @param server The PDU dispatcher; held by reference
     * @param config Unit id; defaults answer any unit
     */
    explicit Protocol(Server& server, const ProtocolConfig& config = {})
        : server(server), config(config)
    {
    }

    /**
     * @brief Size of the ADU at the front of `bytes`, from its MBAP header
     */
    FrameStatus frame_size(std::span<const uint8_t> bytes, uint16_t& length);

    /**
     * @brief Unwrap one ADU, dispatch the PDU, wrap the response
     * @return Response ADU length, or 0 for no response
     */
    uint16_t handle(std::span<const uint8_t> frame,
                    std::span<uint8_t> response);

    const ProtocolStats& statistics() const
    {
        return stats;
    }

private:
    Server& server;
    ProtocolConfig config;
    ProtocolStats stats{};
};

}  // namespace EoT::Net::Modbus
