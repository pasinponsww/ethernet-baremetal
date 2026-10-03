/**
 * @file modbus.cc
 * @brief Name lookups for the Modbus vocabulary
 * @note Diagnostics only; the protocol path never calls these.
 *
 * @author Bex Saw
 */

#include "modbus/modbus.h"

namespace EoT::Net::Modbus
{

const char* to_string(ExceptionCode code)
{
    switch (code)
    {
        case ExceptionCode::IllegalFunction:
            return "illegal function";

        case ExceptionCode::IllegalDataAddress:
            return "illegal data address";

        case ExceptionCode::IllegalDataValue:
            return "illegal data value";

        case ExceptionCode::ServerDeviceFailure:
            return "server device failure";

        default:
            return "unknown";
    }
}

const char* to_string(FunctionCode code)
{
    switch (code)
    {
        case FunctionCode::ReadCoils:
            return "read coils";

        case FunctionCode::ReadDiscreteInputs:
            return "read discrete inputs";

        case FunctionCode::ReadHoldingRegisters:
            return "read holding registers";

        case FunctionCode::ReadInputRegisters:
            return "read input registers";

        case FunctionCode::WriteSingleCoil:
            return "write single coil";

        case FunctionCode::WriteSingleRegister:
            return "write single register";

        case FunctionCode::WriteMultipleCoils:
            return "write multiple coils";

        case FunctionCode::WriteMultipleRegisters:
            return "write multiple registers";

        default:
            return "unsupported";
    }
}

}  // namespace EoT::Net::Modbus
