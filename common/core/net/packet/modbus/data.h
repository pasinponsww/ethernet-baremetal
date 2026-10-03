/**
 * @file data.h
 * @brief Modbus data model and request dispatcher
 * @note Turns a request PDU into a response PDU over caller-owned buffers.
 *       Knows nothing about networks; Modbus::Protocol (protocol.h)
 *       unwraps the ADU and hands the PDU here.
 *
 *       The data model is four spans onto arrays the application owns:
 *
 *         coils              read/write bits   FC 1, 5, 15
 *         discrete_inputs    read-only bits    FC 2
 *         holding_registers  read/write words  FC 3, 6, 16
 *         input_registers    read-only words   FC 4
 *
 *       An empty span means "this device has none"; every access to it is
 *       answered with IllegalDataAddress. Bits are stored one per byte and
 *       only packed on the wire.
 *
 *       Ownership: the application owns the arrays and must keep them
 *       alive for as long as the Server is in use; Server holds only the
 *       spans (a DataModel by value). Server never allocates.
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
 * @brief Called after a request has written to the data model
 * @param context DataModel::hook_context, passed back untouched
 * @param function The write function code that ran
 * @param address First address written
 * @param quantity Number of coils or registers written
 * @note Runs on the superloop before the response is sent; keep it short.
 */
using WriteHook = void (*)(void* context, FunctionCode function,
                           uint16_t address, uint16_t quantity);

/**
 * @brief Application data a Modbus master may reach. Filled in by the BSP
 *        or application and handed to Server
 * @param coils Read/write bits, one byte per coil, zero = off
 * @param discrete_inputs Read-only bits, one byte per input
 * @param holding_registers Read/write 16-bit words
 * @param input_registers Read-only 16-bit words
 * @param on_write Optional write notification, may be null
 * @param hook_context Passed back to on_write untouched
 * @note Addresses are zero-based offsets into these spans; the 4xxxx style
 *       numbering some masters display is not on the wire. A span longer
 *       than 65536 entries is legal but unreachable past a 16-bit address.
 */
struct DataModel
{
    std::span<uint8_t> coils{};
    std::span<uint8_t> discrete_inputs{};
    std::span<uint16_t> holding_registers{};
    std::span<uint16_t> input_registers{};
    WriteHook on_write{nullptr};
    void* hook_context{nullptr};
};

/**
 * @brief Dispatcher counters
 * @param requests PDUs dispatched, successful or not
 * @param exceptions Requests answered with an exception response
 * @param malformed Requests dropped without a response (empty PDU)
 */
struct Stats
{
    uint32_t requests{0U};
    uint32_t exceptions{0U};
    uint32_t malformed{0U};
};

class Server
{
public:
    explicit Server(const DataModel& model) : model(model)
    {
    }

    /**
     * @brief Turn one request PDU into one response PDU
     * @param request Function code first; no MBAP header, address or CRC
     * @param[out] response Should be Spec::kMaxPduSize bytes so any legal
     *        response fits
     * @return Response length, or 0 if no response should be sent
     * @note 0 only for an empty request, which has no function code to
     *       build an exception around. Every other failure produces an
     *       exception response. Never blocks or allocates.
     */
    uint16_t process(std::span<const uint8_t> request,
                     std::span<uint8_t> response);

    const Stats& statistics() const
    {
        return stats;
    }

    /**
     * @brief Replace the data model
     * @note Not safe to call from inside a WriteHook.
     */
    void set_data_model(const DataModel& new_model)
    {
        model = new_model;
    }

private:
    // One handler per function code. Each takes the full request PDU
    // (function code included, so offsets match the spec tables), does
    // its own validation and returns the response length written.
    uint16_t read_bits(std::span<const uint8_t> request,
                       std::span<uint8_t> response,
                       std::span<const uint8_t> source);
    uint16_t read_registers(std::span<const uint8_t> request,
                            std::span<uint8_t> response,
                            std::span<const uint16_t> source);
    uint16_t write_single_coil(std::span<const uint8_t> request,
                               std::span<uint8_t> response);
    uint16_t write_single_register(std::span<const uint8_t> request,
                                   std::span<uint8_t> response);
    uint16_t write_multiple_coils(std::span<const uint8_t> request,
                                  std::span<uint8_t> response);
    uint16_t write_multiple_registers(std::span<const uint8_t> request,
                                      std::span<uint8_t> response);

    /**
     * @brief Build an exception response and count it
     * @param function Request function code; the exception flag is added
     * @param code The exception to report
     * @param[out] response Needs 2 bytes
     * @return 2, or 0 if `response` is too small
     */
    uint16_t make_exception(uint8_t function, ExceptionCode code,
                            std::span<uint8_t> response);

    void notify_write(FunctionCode function, uint16_t address,
                      uint16_t quantity);

    DataModel model;
    Stats stats{};
};

}  // namespace EoT::Net::Modbus
