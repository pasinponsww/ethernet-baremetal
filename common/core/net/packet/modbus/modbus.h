/**
 * @file modbus.h
 * @brief Modbus vocabulary shared by every transport
 * @note Constants and enums from the Modbus Application Protocol spec
 *       v1.1b3. Transport independent -- they mean the same thing over
 *       TCP, UDP and serial.
 *
 *       Layout of packet/modbus, innermost first:
 *
 *         modbus.h     function codes, exceptions, Spec constants
 *         data.h       data model + request dispatcher (PDU in, PDU out)
 *         mbap.h       7-byte MBAP header codec (PDU <-> ADU)
 *         protocol.h   Modbus::Protocol, the PacketProtocol the generic
 *                      TcpServer<T> / UdpServer<T> in packet/ carry
 *
 *       Nothing here touches lwIP; it is all byte-in/byte-out and host
 *       testable. The transports live one level up in packet/.
 *
 *         PDU = function code + data, max 253 bytes
 *         ADU = MBAP header + PDU, max 260 bytes
 *
 *       Widths: every length, offset and count in this layer is uint16_t
 *       (or uint8_t where the spec caps it at one byte), never size_t. An
 *       ADU is at most 260 bytes and lwIP hands buffers around as u16_t,
 *       so a wider type would only ever be narrowed at the boundary.
 *       size_t appears only where the standard library forces it
 *       (std::span::size(), std::array extents).
 *
 * @author Bex Saw
 */

#pragma once
#include <cstdint>

namespace EoT::Net::Modbus
{

/**
 * @brief Function codes this implementation serves
 * @note Codes 1, 2, 5, 15 address bits; 3, 4, 6, 16 address 16-bit words.
 *       Anything else gets ExceptionCode::IllegalFunction.
 */
enum class FunctionCode : uint8_t
{
    ReadCoils = 0x01,
    ReadDiscreteInputs = 0x02,
    ReadHoldingRegisters = 0x03,
    ReadInputRegisters = 0x04,
    WriteSingleCoil = 0x05,
    WriteSingleRegister = 0x06,
    WriteMultipleCoils = 0x0F,
    WriteMultipleRegisters = 0x10
};

/**
 * @brief Exception codes returned in an error response
 *        IllegalFunction     -> function code not supported
 *        IllegalDataAddress  -> address range outside the data model
 *        IllegalDataValue    -> quantity or value field out of range
 *        ServerDeviceFailure -> well formed, but could not be carried out
 */
enum class ExceptionCode : uint8_t
{
    IllegalFunction = 0x01,
    IllegalDataAddress = 0x02,
    IllegalDataValue = 0x03,
    ServerDeviceFailure = 0x04
};

/**
 * @brief Protocol constants fixed by the spec
 * @param kMaxPduSize Largest PDU, function code included
 * @param kMbapHeaderSize MBAP header preceding the PDU on TCP and UDP
 * @param kMaxAduSize MBAP header plus a full PDU
 * @param kDefaultPort Registered port for Modbus TCP (and by convention UDP)
 * @param kProtocolId MBAP protocol identifier, always 0 for Modbus
 * @param kDefaultUnitId Unit id meaning "the device at this IP address".
 *        Masters also send 0 and 1, so a Protocol configured with this
 *        value answers for any unit id
 * @param kExceptionFlag OR'd into the response function code on error
 * @param kMaxReadBits Bits per read request (FC 1, 2)
 * @param kMaxReadRegisters Registers per read request (FC 3, 4)
 * @param kMaxWriteBits Bits per write request (FC 15)
 * @param kMaxWriteRegisters Registers per write request (FC 16)
 * @param kCoilOn WriteSingleCoil value meaning ON; FC 5 takes exactly this
 *        or kCoilOff, anything else is IllegalDataValue
 * @param kCoilOff WriteSingleCoil value meaning OFF
 */
struct Spec
{
    static constexpr uint16_t kMaxPduSize{253U};
    static constexpr uint16_t kMbapHeaderSize{7U};
    static constexpr uint16_t kMaxAduSize{kMbapHeaderSize + kMaxPduSize};
    static constexpr uint16_t kDefaultPort{502U};
    static constexpr uint16_t kProtocolId{0U};
    static constexpr uint8_t kDefaultUnitId{0xFFU};
    static constexpr uint8_t kExceptionFlag{0x80U};
    static constexpr uint16_t kMaxReadBits{2000U};
    static constexpr uint16_t kMaxReadRegisters{125U};
    static constexpr uint16_t kMaxWriteBits{1968U};
    static constexpr uint16_t kMaxWriteRegisters{123U};
    static constexpr uint16_t kCoilOn{0xFF00U};
    static constexpr uint16_t kCoilOff{0x0000U};
};

/**
 * @brief Byte geometry used by the codec and the bit packers
 * @param kBitsPerByte Shift between the two halves of a 16-bit field, and
 *        the packing ratio for coil data
 * @param kByteMask Low byte of a 16-bit field
 */
struct Wire
{
    static constexpr uint8_t kBitsPerByte{8U};
    static constexpr uint16_t kByteMask{0xFFU};
};

/*                                                                */
/*                      Byte-order helpers                        */
/*                                                                */

// Modbus is big-endian on the wire. These are the only place byte order
// is handled. Hand-written because C++20 has no byte-array <-> integer
// endian conversion: std::endian only reports the host order, and
// std::byteswap is C++23.

/**
 * @brief Read a big-endian 16-bit value
 * @param bytes Pointer to the high byte; two bytes must be readable
 */
constexpr uint16_t read_be16(const uint8_t* bytes)
{
    return static_cast<uint16_t>(
        (static_cast<uint16_t>(bytes[0]) << Wire::kBitsPerByte) |
        static_cast<uint16_t>(bytes[1]));
}

/**
 * @brief Write a 16-bit value as big-endian
 * @param bytes Destination for the high byte; two bytes are written
 * @param value Value to write
 */
constexpr void write_be16(uint8_t* bytes, uint16_t value)
{
    bytes[0] =
        static_cast<uint8_t>((value >> Wire::kBitsPerByte) & Wire::kByteMask);
    bytes[1] = static_cast<uint8_t>(value & Wire::kByteMask);
}

/*                                                                */
/*                      Diagnostics                               */
/*                                                                */

// Enum-to-name lookups for logging. Not in <string>/<format>: the standard
// library has no enum reflection before C++26, and std::to_string only
// formats numbers. The protocol path never calls these.

/**
 * @brief Name of an exception code
 * @return Static string; "unknown" for an unrecognized code
 */
const char* to_string(ExceptionCode code);

/**
 * @brief Name of a function code
 * @return Static string; "unsupported" for anything not served
 */
const char* to_string(FunctionCode code);

}  // namespace EoT::Net::Modbus
