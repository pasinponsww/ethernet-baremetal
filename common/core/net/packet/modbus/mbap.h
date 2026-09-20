/**
 * @file mbap.h
 * @brief MBAP header codec: wraps a PDU into a Modbus TCP/IP ADU
 * @note Seven bytes in front of every PDU on TCP or UDP, replacing the
 *       address and CRC serial Modbus uses:
 *
 *         offset  size  field
 *         0       2     transaction id  echoed back so a master can match
 *                                       responses to requests
 *         2       2     protocol id     always 0
 *         4       2     length          bytes AFTER this field: unit id +
 *                                       PDU, so a 1-byte PDU gives 2
 *         6       1     unit id         slave address, or 0xFF
 *
 *       The length field is the only framing a TCP reader has, and
 *       counting it from the wrong base is the classic Modbus TCP bug, so
 *       encode/decode keep it in one place.
 *
 *       Ownership: pure functions over caller-owned spans. Nothing is
 *       allocated or retained.
 *
 * @author Bex Saw
 */

#pragma once
#include <cstdint>
#include <span>
#include "modbus/modbus.h"

namespace EoT::Net::Modbus
{

/**
 * @brief Decoded MBAP header
 * @param transaction_id Echoed verbatim in the response
 * @param protocol_id Must be Spec::kProtocolId
 * @param length Byte count of unit id + PDU, as carried on the wire
 * @param unit_id Slave address, or Spec::kDefaultUnitId
 */
struct MbapHeader
{
    uint16_t transaction_id{0U};
    uint16_t protocol_id{Spec::kProtocolId};
    uint16_t length{0U};
    uint8_t unit_id{Spec::kDefaultUnitId};

    /**
     * @brief PDU size this header claims follows it
     * @return length - 1, or 0 when length is 0 (malformed)
     */
    uint16_t pdu_size() const
    {
        return (length == 0U) ? 0U : static_cast<uint16_t>(length - 1U);
    }
};

/**
 * @brief Result of decode_mbap
 *        Ok             -> header is well formed
 *        Incomplete     -> fewer than 7 bytes; on TCP, wait for more
 *        BadProtocolId  -> not Modbus
 *        BadLength      -> length is 0 or implies a PDU over
 *                          Spec::kMaxPduSize
 */
enum class MbapStatus : uint8_t
{
    Ok = 0,
    Incomplete,
    BadProtocolId,
    BadLength
};

/**
 * @brief Decode and validate an MBAP header
 * @param bytes Buffer positioned at the first header byte
 * @param[out] out Filled only on MbapStatus::Ok
 * @note Does not look at the PDU, which on TCP may not have arrived yet.
 */
MbapStatus decode_mbap(std::span<const uint8_t> bytes, MbapHeader& out);

/**
 * @brief Write an MBAP header for a response PDU
 * @param header Transaction id and unit id to echo; its length field is
 *        ignored and recomputed from pdu_size
 * @param pdu_size Size of the response PDU that follows
 * @param[out] out Needs Spec::kMbapHeaderSize bytes
 * @return true if exactly Spec::kMbapHeaderSize bytes were written; false
 *         (nothing written) if `out` is too small or pdu_size is not a
 *         legal PDU size
 */
bool encode_mbap(const MbapHeader& header, uint16_t pdu_size,
                 std::span<uint8_t> out);

}  // namespace EoT::Net::Modbus
