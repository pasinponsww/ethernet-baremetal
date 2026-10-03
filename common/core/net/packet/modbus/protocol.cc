/**
 * @file protocol.cc
 * @brief Modbus TCP/IP framing implementation
 *
 * @author Bex Saw
 */

#include "modbus/protocol.h"

namespace EoT::Net::Modbus
{

FrameStatus Protocol::frame_size(std::span<const uint8_t> bytes,
                                 uint16_t& length)
{
    MbapHeader header{};

    switch (decode_mbap(bytes, header))
    {
        case MbapStatus::Ok:
            break;

        case MbapStatus::Incomplete:
            return FrameStatus::Incomplete;

        default:
            return FrameStatus::Invalid;
    }

    const uint16_t adu_size{
        static_cast<uint16_t>(Spec::kMbapHeaderSize + header.pdu_size())};

    if (bytes.size() < adu_size)
    {
        // Header is here, body is still in flight.
        return FrameStatus::Incomplete;
    }

    length = adu_size;
    return FrameStatus::Complete;
}

uint16_t Protocol::handle(std::span<const uint8_t> frame,
                          std::span<uint8_t> response)
{
    MbapHeader header{};

    // The transport already sized this frame via frame_size, so the header
    // decodes; re-decoding is cheaper than carrying it across the seam.
    if (decode_mbap(frame, header) != MbapStatus::Ok ||
        frame.size() <
            static_cast<uint16_t>(Spec::kMbapHeaderSize + header.pdu_size()) ||
        response.size() < Spec::kMbapHeaderSize)
    {
        return 0U;
    }

    if (config.unit_id != Spec::kDefaultUnitId &&
        header.unit_id != config.unit_id)
    {
        // Addressed to another device behind a gateway.
        ++stats.filtered;
        return 0U;
    }

    const std::span<const uint8_t> request{
        frame.subspan(Spec::kMbapHeaderSize, header.pdu_size())};
    const std::span<uint8_t> response_pdu{
        response.subspan(Spec::kMbapHeaderSize)};

    const uint16_t response_size{server.process(request, response_pdu)};

    ++stats.requests;

    if (response_size == 0U)
    {
        ++stats.silent;
        return 0U;
    }

    // Echo transaction and unit id so the master can match the response.
    MbapHeader response_header{};
    response_header.transaction_id = header.transaction_id;
    response_header.unit_id = header.unit_id;

    if (!encode_mbap(response_header, response_size,
                     response.first(Spec::kMbapHeaderSize)))
    {
        return 0U;
    }

    return static_cast<uint16_t>(Spec::kMbapHeaderSize + response_size);
}

}  // namespace EoT::Net::Modbus
