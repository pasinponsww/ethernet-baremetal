/**
 * @file data.cc
 * @brief Modbus request dispatcher implementation
 * @note Every handler: check the request is long enough to read its
 *       fields, check the fields are in range, then act. That order is
 *       what keeps a short PDU from becoming an out-of-bounds read.
 *
 *       Range checks widen address + quantity to uint32_t before adding
 *       (at most 65535 + 2000, which does not fit uint16_t) and compare
 *       against the span size.
 *
 * @author Bex Saw
 */

#include "modbus/data.h"

#include <algorithm>  // std::copy_n, std::fill_n

namespace EoT::Net::Modbus
{

namespace
{

// Request PDU field offsets, function code at 0.
constexpr uint8_t kFunctionOffset{0U};
constexpr uint8_t kAddressOffset{1U};
constexpr uint8_t kQuantityOffset{3U};
constexpr uint8_t kByteCountOffset{5U};
constexpr uint8_t kWriteDataOffset{6U};

// Function code + address/quantity pair.
constexpr uint8_t kAddressQuantityRequestSize{5U};

// FC 15 and 16 add a byte count before the payload.
constexpr uint8_t kMultiWriteHeaderSize{6U};

// Function code + exception code.
constexpr uint8_t kExceptionResponseSize{2U};

// Function code + byte count, ahead of read data.
constexpr uint8_t kReadResponseHeaderSize{2U};

constexpr uint16_t packed_byte_count(uint16_t bit_count)
{
    return static_cast<uint16_t>((bit_count + Wire::kBitsPerByte - 1U) /
                                 Wire::kBitsPerByte);
}

constexpr bool in_range(uint16_t address, uint16_t quantity, size_t size)
{
    return static_cast<uint32_t>(address) + quantity <= size;
}

}  // namespace

uint16_t Server::make_exception(uint8_t function, ExceptionCode code,
                                std::span<uint8_t> response)
{
    if (response.size() < kExceptionResponseSize)
    {
        return 0U;
    }

    ++stats.exceptions;

    response[0] = static_cast<uint8_t>(function | Spec::kExceptionFlag);
    response[1] = static_cast<uint8_t>(code);

    return kExceptionResponseSize;
}

void Server::notify_write(FunctionCode function, uint16_t address,
                          uint16_t quantity)
{
    if (model.on_write != nullptr)
    {
        model.on_write(model.hook_context, function, address, quantity);
    }
}

uint16_t Server::process(std::span<const uint8_t> request,
                         std::span<uint8_t> response)
{
    // No function code to echo in an exception, so the spec says stay
    // silent.
    if (request.empty())
    {
        ++stats.malformed;
        return 0U;
    }

    ++stats.requests;

    const uint8_t function{request[kFunctionOffset]};

    switch (static_cast<FunctionCode>(function))
    {
        case FunctionCode::ReadCoils:
            return read_bits(request, response, model.coils);

        case FunctionCode::ReadDiscreteInputs:
            return read_bits(request, response, model.discrete_inputs);

        case FunctionCode::ReadHoldingRegisters:
            return read_registers(request, response, model.holding_registers);

        case FunctionCode::ReadInputRegisters:
            return read_registers(request, response, model.input_registers);

        case FunctionCode::WriteSingleCoil:
            return write_single_coil(request, response);

        case FunctionCode::WriteSingleRegister:
            return write_single_register(request, response);

        case FunctionCode::WriteMultipleCoils:
            return write_multiple_coils(request, response);

        case FunctionCode::WriteMultipleRegisters:
            return write_multiple_registers(request, response);

        default:
            // An answer, not silence: a master needs to tell "unsupported"
            // from "nobody home".
            return make_exception(function, ExceptionCode::IllegalFunction,
                                  response);
    }
}

uint16_t Server::read_bits(std::span<const uint8_t> request,
                           std::span<uint8_t> response,
                           std::span<const uint8_t> source)
{
    const uint8_t function{request[kFunctionOffset]};

    if (request.size() < kAddressQuantityRequestSize)
    {
        return make_exception(function, ExceptionCode::IllegalDataValue,
                              response);
    }

    const uint16_t address{read_be16(&request[kAddressOffset])};
    const uint16_t quantity{read_be16(&request[kQuantityOffset])};

    if (quantity == 0U || quantity > Spec::kMaxReadBits)
    {
        return make_exception(function, ExceptionCode::IllegalDataValue,
                              response);
    }

    if (!in_range(address, quantity, source.size()))
    {
        return make_exception(function, ExceptionCode::IllegalDataAddress,
                              response);
    }

    const uint16_t byte_count{packed_byte_count(quantity)};
    const uint16_t response_size{
        static_cast<uint16_t>(kReadResponseHeaderSize + byte_count)};

    if (response.size() < response_size)
    {
        return make_exception(function, ExceptionCode::ServerDeviceFailure,
                              response);
    }

    response[0] = function;
    response[1] = static_cast<uint8_t>(byte_count);

    uint8_t* const data{&response[kReadResponseHeaderSize]};
    std::fill_n(data, byte_count, 0U);

    // Pack LSB first within each byte: the first requested coil is bit 0
    // of the first data byte.
    for (uint16_t i = 0U; i < quantity; ++i)
    {
        if (source[static_cast<size_t>(address) + i] != 0U)
        {
            data[i / Wire::kBitsPerByte] |=
                static_cast<uint8_t>(1U << (i % Wire::kBitsPerByte));
        }
    }

    return response_size;
}

uint16_t Server::read_registers(std::span<const uint8_t> request,
                                std::span<uint8_t> response,
                                std::span<const uint16_t> source)
{
    const uint8_t function{request[kFunctionOffset]};

    if (request.size() < kAddressQuantityRequestSize)
    {
        return make_exception(function, ExceptionCode::IllegalDataValue,
                              response);
    }

    const uint16_t address{read_be16(&request[kAddressOffset])};
    const uint16_t quantity{read_be16(&request[kQuantityOffset])};

    if (quantity == 0U || quantity > Spec::kMaxReadRegisters)
    {
        return make_exception(function, ExceptionCode::IllegalDataValue,
                              response);
    }

    if (!in_range(address, quantity, source.size()))
    {
        return make_exception(function, ExceptionCode::IllegalDataAddress,
                              response);
    }

    const uint16_t byte_count{static_cast<uint16_t>(quantity * 2U)};
    const uint16_t response_size{
        static_cast<uint16_t>(kReadResponseHeaderSize + byte_count)};

    if (response.size() < response_size)
    {
        return make_exception(function, ExceptionCode::ServerDeviceFailure,
                              response);
    }

    response[0] = function;
    response[1] = static_cast<uint8_t>(byte_count);

    uint8_t* const data{&response[kReadResponseHeaderSize]};

    for (uint16_t i = 0U; i < quantity; ++i)
    {
        write_be16(&data[i * 2U], source[static_cast<size_t>(address) + i]);
    }

    return response_size;
}

uint16_t Server::write_single_coil(std::span<const uint8_t> request,
                                   std::span<uint8_t> response)
{
    const uint8_t function{request[kFunctionOffset]};

    if (request.size() < kAddressQuantityRequestSize)
    {
        return make_exception(function, ExceptionCode::IllegalDataValue,
                              response);
    }

    const uint16_t address{read_be16(&request[kAddressOffset])};

    // FC 5 carries one of two magic constants, not a boolean.
    const uint16_t value{read_be16(&request[kQuantityOffset])};

    if (value != Spec::kCoilOn && value != Spec::kCoilOff)
    {
        return make_exception(function, ExceptionCode::IllegalDataValue,
                              response);
    }

    if (address >= model.coils.size())
    {
        return make_exception(function, ExceptionCode::IllegalDataAddress,
                              response);
    }

    model.coils[address] = (value == Spec::kCoilOn) ? 1U : 0U;
    notify_write(FunctionCode::WriteSingleCoil, address, 1U);

    // A successful single write echoes the request back byte for byte.
    if (response.size() < kAddressQuantityRequestSize)
    {
        return 0U;
    }

    std::copy_n(request.data(), kAddressQuantityRequestSize, response.data());

    return kAddressQuantityRequestSize;
}

uint16_t Server::write_single_register(std::span<const uint8_t> request,
                                       std::span<uint8_t> response)
{
    const uint8_t function{request[kFunctionOffset]};

    if (request.size() < kAddressQuantityRequestSize)
    {
        return make_exception(function, ExceptionCode::IllegalDataValue,
                              response);
    }

    const uint16_t address{read_be16(&request[kAddressOffset])};
    const uint16_t value{read_be16(&request[kQuantityOffset])};

    if (address >= model.holding_registers.size())
    {
        return make_exception(function, ExceptionCode::IllegalDataAddress,
                              response);
    }

    model.holding_registers[address] = value;
    notify_write(FunctionCode::WriteSingleRegister, address, 1U);

    if (response.size() < kAddressQuantityRequestSize)
    {
        return 0U;
    }

    std::copy_n(request.data(), kAddressQuantityRequestSize, response.data());

    return kAddressQuantityRequestSize;
}

uint16_t Server::write_multiple_coils(std::span<const uint8_t> request,
                                      std::span<uint8_t> response)
{
    const uint8_t function{request[kFunctionOffset]};

    if (request.size() < kMultiWriteHeaderSize)
    {
        return make_exception(function, ExceptionCode::IllegalDataValue,
                              response);
    }

    const uint16_t address{read_be16(&request[kAddressOffset])};
    const uint16_t quantity{read_be16(&request[kQuantityOffset])};
    const uint8_t byte_count{request[kByteCountOffset]};

    // Quantity legal, byte count matches it, and the PDU actually carries
    // that many payload bytes.
    if (quantity == 0U || quantity > Spec::kMaxWriteBits ||
        byte_count != packed_byte_count(quantity) ||
        request.size() <
            static_cast<size_t>(kMultiWriteHeaderSize) + byte_count)
    {
        return make_exception(function, ExceptionCode::IllegalDataValue,
                              response);
    }

    if (!in_range(address, quantity, model.coils.size()))
    {
        return make_exception(function, ExceptionCode::IllegalDataAddress,
                              response);
    }

    const uint8_t* const data{&request[kWriteDataOffset]};

    // Unpack one bit per coil to one byte per coil, LSB first.
    for (uint16_t i = 0U; i < quantity; ++i)
    {
        const uint8_t packed{data[i / Wire::kBitsPerByte]};
        const bool bit{(packed & static_cast<uint8_t>(
                                     1U << (i % Wire::kBitsPerByte))) != 0U};

        model.coils[static_cast<size_t>(address) + i] = bit ? 1U : 0U;
    }

    notify_write(FunctionCode::WriteMultipleCoils, address, quantity);

    // Response is address and quantity only, not the data.
    if (response.size() < kAddressQuantityRequestSize)
    {
        return 0U;
    }

    response[0] = function;
    write_be16(&response[kAddressOffset], address);
    write_be16(&response[kQuantityOffset], quantity);

    return kAddressQuantityRequestSize;
}

uint16_t Server::write_multiple_registers(std::span<const uint8_t> request,
                                          std::span<uint8_t> response)
{
    const uint8_t function{request[kFunctionOffset]};

    if (request.size() < kMultiWriteHeaderSize)
    {
        return make_exception(function, ExceptionCode::IllegalDataValue,
                              response);
    }

    const uint16_t address{read_be16(&request[kAddressOffset])};
    const uint16_t quantity{read_be16(&request[kQuantityOffset])};
    const uint8_t byte_count{request[kByteCountOffset]};

    if (quantity == 0U || quantity > Spec::kMaxWriteRegisters ||
        byte_count != quantity * 2U ||
        request.size() <
            static_cast<size_t>(kMultiWriteHeaderSize) + byte_count)
    {
        return make_exception(function, ExceptionCode::IllegalDataValue,
                              response);
    }

    if (!in_range(address, quantity, model.holding_registers.size()))
    {
        return make_exception(function, ExceptionCode::IllegalDataAddress,
                              response);
    }

    const uint8_t* const data{&request[kWriteDataOffset]};

    for (uint16_t i = 0U; i < quantity; ++i)
    {
        model.holding_registers[static_cast<size_t>(address) + i] =
            read_be16(&data[i * 2U]);
    }

    notify_write(FunctionCode::WriteMultipleRegisters, address, quantity);

    if (response.size() < kAddressQuantityRequestSize)
    {
        return 0U;
    }

    response[0] = function;
    write_be16(&response[kAddressOffset], address);
    write_be16(&response[kQuantityOffset], quantity);

    return kAddressQuantityRequestSize;
}

}  // namespace EoT::Net::Modbus
