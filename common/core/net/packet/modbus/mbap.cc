/**
 * @file mbap.cc
 * @brief MBAP header codec implementation
 *
 * @author Bex Saw
 */

#include "modbus/mbap.h"

namespace EoT::Net::Modbus
{

namespace
{

// Field offsets within the 7-byte header.
constexpr uint8_t kTransactionIdOffset{0U};
constexpr uint8_t kProtocolIdOffset{2U};
constexpr uint8_t kLengthOffset{4U};
constexpr uint8_t kUnitIdOffset{6U};

}  // namespace

MbapStatus decode_mbap(std::span<const uint8_t> bytes, MbapHeader& out)
{
    if (bytes.size() < Spec::kMbapHeaderSize)
    {
        return MbapStatus::Incomplete;
    }

    MbapHeader header{};
    header.transaction_id = read_be16(&bytes[kTransactionIdOffset]);
    header.protocol_id = read_be16(&bytes[kProtocolIdOffset]);
    header.length = read_be16(&bytes[kLengthOffset]);
    header.unit_id = bytes[kUnitIdOffset];

    if (header.protocol_id != Spec::kProtocolId)
    {
        return MbapStatus::BadProtocolId;
    }

    // Upper bound checked here so a hostile length field can never size a
    // read later on.
    if (header.length == 0U || header.pdu_size() > Spec::kMaxPduSize)
    {
        return MbapStatus::BadLength;
    }

    out = header;
    return MbapStatus::Ok;
}

bool encode_mbap(const MbapHeader& header, uint16_t pdu_size,
                 std::span<uint8_t> out)
{
    if (out.size() < Spec::kMbapHeaderSize)
    {
        return false;
    }

    if (pdu_size == 0U || pdu_size > Spec::kMaxPduSize)
    {
        return false;
    }

    write_be16(&out[kTransactionIdOffset], header.transaction_id);
    write_be16(&out[kProtocolIdOffset], Spec::kProtocolId);

    // length = everything after the length field: unit id + PDU.
    write_be16(&out[kLengthOffset], static_cast<uint16_t>(pdu_size + 1U));

    out[kUnitIdOffset] = header.unit_id;

    return true;
}

}  // namespace EoT::Net::Modbus
