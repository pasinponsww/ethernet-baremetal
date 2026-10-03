/**
 * @file data_test.cc
 * @brief PDU dispatcher: each function code's happy path, bounds and
 *        exception responses
 *
 * @author Bex Saw
 */

#include <array>
#include <cstdint>
#include <span>
#include <vector>

#include <gtest/gtest.h>

#include "modbus/data.h"

namespace
{

using namespace EoT::Net::Modbus;

/**
 * @brief A small data model with every table populated, plus a write hook
 *        that records what it was told
 */
class DataTest : public ::testing::Test
{
protected:
    static void on_write(void* context, FunctionCode function, uint16_t address,
                         uint16_t quantity)
    {
        auto* self{static_cast<DataTest*>(context)};
        self->hook_calls.push_back({function, address, quantity});
    }

    void SetUp() override
    {
        for (size_t i = 0U; i < coils.size(); ++i)
        {
            coils[i] = static_cast<uint8_t>(i % 2U);  // 0,1,0,1,...
        }

        for (size_t i = 0U; i < discrete_inputs.size(); ++i)
        {
            discrete_inputs[i] = static_cast<uint8_t>((i % 3U) == 0U);
        }

        for (size_t i = 0U; i < holding_registers.size(); ++i)
        {
            holding_registers[i] = static_cast<uint16_t>(0x1000U + i);
        }

        for (size_t i = 0U; i < input_registers.size(); ++i)
        {
            input_registers[i] = static_cast<uint16_t>(0x2000U + i);
        }

        model.coils = coils;
        model.discrete_inputs = discrete_inputs;
        model.holding_registers = holding_registers;
        model.input_registers = input_registers;
        model.on_write = &DataTest::on_write;
        model.hook_context = this;

        server.set_data_model(model);
    }

    uint16_t process(std::span<const uint8_t> request)
    {
        return server.process(request, response);
    }

    struct HookCall
    {
        FunctionCode function;
        uint16_t address;
        uint16_t quantity;
    };

    std::array<uint8_t, 16> coils{};
    std::array<uint8_t, 16> discrete_inputs{};
    std::array<uint16_t, 8> holding_registers{};
    std::array<uint16_t, 8> input_registers{};
    DataModel model{};
    Server server{model};
    std::array<uint8_t, Spec::kMaxPduSize> response{};
    std::vector<HookCall> hook_calls{};
};

TEST_F(DataTest, EmptyRequestIsSilentAndCountedMalformed)
{
    EXPECT_EQ(process(std::span<const uint8_t>{}), 0U);
    EXPECT_EQ(server.statistics().malformed, 1U);
    EXPECT_EQ(server.statistics().requests, 0U);
}

TEST_F(DataTest, UnknownFunctionGetsIllegalFunction)
{
    const std::array<uint8_t, 5> request{0x2B, 0x00, 0x00, 0x00, 0x01};

    ASSERT_EQ(process(request), 2U);
    EXPECT_EQ(response[0], 0x2B | Spec::kExceptionFlag);
    EXPECT_EQ(response[1],
              static_cast<uint8_t>(ExceptionCode::IllegalFunction));
    EXPECT_EQ(server.statistics().exceptions, 1U);
}

TEST_F(DataTest, ReadCoilsPacksLsbFirst)
{
    // FC1, address 0, quantity 10 -> 2 data bytes
    const std::array<uint8_t, 5> request{0x01, 0x00, 0x00, 0x00, 0x0A};

    ASSERT_EQ(process(request), 4U);
    EXPECT_EQ(response[0], 0x01);
    EXPECT_EQ(response[1], 2U);
    EXPECT_EQ(response[2], 0xAA);  // coils 1,3,5,7 set -> 0b10101010
    EXPECT_EQ(response[3], 0x02);  // coils 8,9 -> only 9 set -> 0b00000010
}

TEST_F(DataTest, ReadCoilsPastEndIsIllegalDataAddress)
{
    // address 10, quantity 7: 10 + 7 > 16
    const std::array<uint8_t, 5> request{0x01, 0x00, 0x0A, 0x00, 0x07};

    ASSERT_EQ(process(request), 2U);
    EXPECT_EQ(response[0], 0x81);
    EXPECT_EQ(response[1],
              static_cast<uint8_t>(ExceptionCode::IllegalDataAddress));
}

TEST_F(DataTest, ReadCoilsZeroQuantityIsIllegalDataValue)
{
    const std::array<uint8_t, 5> request{0x01, 0x00, 0x00, 0x00, 0x00};

    ASSERT_EQ(process(request), 2U);
    EXPECT_EQ(response[1],
              static_cast<uint8_t>(ExceptionCode::IllegalDataValue));
}

TEST_F(DataTest, ReadCoilsShortRequestIsIllegalDataValue)
{
    const std::array<uint8_t, 3> request{0x01, 0x00, 0x00};

    ASSERT_EQ(process(request), 2U);
    EXPECT_EQ(response[0], 0x81);
    EXPECT_EQ(response[1],
              static_cast<uint8_t>(ExceptionCode::IllegalDataValue));
}

TEST_F(DataTest, ReadDiscreteInputs)
{
    const std::array<uint8_t, 5> request{0x02, 0x00, 0x00, 0x00, 0x08};

    ASSERT_EQ(process(request), 3U);
    EXPECT_EQ(response[0], 0x02);
    EXPECT_EQ(response[1], 1U);
    EXPECT_EQ(response[2], 0x49);  // inputs 0,3,6 set -> 0b01001001
}

TEST_F(DataTest, ReadHoldingRegistersIsBigEndian)
{
    // FC3, address 2, quantity 2
    const std::array<uint8_t, 5> request{0x03, 0x00, 0x02, 0x00, 0x02};

    ASSERT_EQ(process(request), 6U);
    EXPECT_EQ(response[0], 0x03);
    EXPECT_EQ(response[1], 4U);
    EXPECT_EQ(response[2], 0x10);
    EXPECT_EQ(response[3], 0x02);
    EXPECT_EQ(response[4], 0x10);
    EXPECT_EQ(response[5], 0x03);
}

TEST_F(DataTest, ReadInputRegisters)
{
    const std::array<uint8_t, 5> request{0x04, 0x00, 0x07, 0x00, 0x01};

    ASSERT_EQ(process(request), 4U);
    EXPECT_EQ(response[2], 0x20);
    EXPECT_EQ(response[3], 0x07);
}

TEST_F(DataTest, ReadRegistersOverSpecLimitIsIllegalDataValue)
{
    std::array<uint8_t, 5> request{0x03, 0x00, 0x00, 0x00, 0x00};
    write_be16(&request[3],
               static_cast<uint16_t>(Spec::kMaxReadRegisters + 1U));

    ASSERT_EQ(process(request), 2U);
    EXPECT_EQ(response[1],
              static_cast<uint8_t>(ExceptionCode::IllegalDataValue));
}

TEST_F(DataTest, EmptyTableAlwaysIllegalDataAddress)
{
    model.input_registers = {};
    server.set_data_model(model);

    const std::array<uint8_t, 5> request{0x04, 0x00, 0x00, 0x00, 0x01};

    ASSERT_EQ(process(request), 2U);
    EXPECT_EQ(response[1],
              static_cast<uint8_t>(ExceptionCode::IllegalDataAddress));
}

TEST_F(DataTest, WriteSingleCoilOnEchoesRequestAndNotifies)
{
    const std::array<uint8_t, 5> request{0x05, 0x00, 0x04, 0xFF, 0x00};

    ASSERT_EQ(process(request), 5U);
    EXPECT_TRUE(std::equal(request.begin(), request.end(), response.begin()));
    EXPECT_EQ(coils[4], 1U);

    ASSERT_EQ(hook_calls.size(), 1U);
    EXPECT_EQ(hook_calls[0].function, FunctionCode::WriteSingleCoil);
    EXPECT_EQ(hook_calls[0].address, 4U);
    EXPECT_EQ(hook_calls[0].quantity, 1U);
}

TEST_F(DataTest, WriteSingleCoilOff)
{
    const std::array<uint8_t, 5> request{0x05, 0x00, 0x01, 0x00, 0x00};

    ASSERT_EQ(process(request), 5U);
    EXPECT_EQ(coils[1], 0U);
}

TEST_F(DataTest, WriteSingleCoilRejectsNonMagicValue)
{
    const std::array<uint8_t, 5> request{0x05, 0x00, 0x01, 0x00, 0x01};

    ASSERT_EQ(process(request), 2U);
    EXPECT_EQ(response[0], 0x85);
    EXPECT_EQ(response[1],
              static_cast<uint8_t>(ExceptionCode::IllegalDataValue));
    EXPECT_EQ(coils[1], 1U);  // untouched
    EXPECT_TRUE(hook_calls.empty());
}

TEST_F(DataTest, WriteSingleRegister)
{
    const std::array<uint8_t, 5> request{0x06, 0x00, 0x03, 0xAB, 0xCD};

    ASSERT_EQ(process(request), 5U);
    EXPECT_EQ(holding_registers[3], 0xABCDU);
    ASSERT_EQ(hook_calls.size(), 1U);
    EXPECT_EQ(hook_calls[0].function, FunctionCode::WriteSingleRegister);
}

TEST_F(DataTest, WriteSingleRegisterPastEnd)
{
    const std::array<uint8_t, 5> request{0x06, 0x00, 0x08, 0x00, 0x00};

    ASSERT_EQ(process(request), 2U);
    EXPECT_EQ(response[1],
              static_cast<uint8_t>(ExceptionCode::IllegalDataAddress));
}

TEST_F(DataTest, WriteMultipleCoilsUnpacksLsbFirst)
{
    // FC15, address 0, quantity 10, byte count 2, data 0x0F 0x03
    const std::array<uint8_t, 8> request{0x0F, 0x00, 0x00, 0x00,
                                         0x0A, 0x02, 0x0F, 0x03};

    ASSERT_EQ(process(request), 5U);
    EXPECT_EQ(response[0], 0x0F);
    EXPECT_EQ(response[4], 0x0A);  // echoes quantity, not data

    const std::array<uint8_t, 10> expected{1, 1, 1, 1, 0, 0, 0, 0, 1, 1};
    for (size_t i = 0U; i < expected.size(); ++i)
    {
        EXPECT_EQ(coils[i], expected[i]) << "coil " << i;
    }

    EXPECT_EQ(coils[10], 0U);  // beyond the write, untouched
    ASSERT_EQ(hook_calls.size(), 1U);
    EXPECT_EQ(hook_calls[0].quantity, 10U);
}

TEST_F(DataTest, WriteMultipleCoilsByteCountMismatch)
{
    // quantity 10 needs 2 bytes, claims 1
    const std::array<uint8_t, 7> request{0x0F, 0x00, 0x00, 0x00,
                                         0x0A, 0x01, 0xFF};

    ASSERT_EQ(process(request), 2U);
    EXPECT_EQ(response[1],
              static_cast<uint8_t>(ExceptionCode::IllegalDataValue));
}

TEST_F(DataTest, WriteMultipleCoilsTruncatedPayload)
{
    // claims 2 data bytes, carries 1
    const std::array<uint8_t, 7> request{0x0F, 0x00, 0x00, 0x00,
                                         0x0A, 0x02, 0xFF};

    ASSERT_EQ(process(request), 2U);
    EXPECT_EQ(response[1],
              static_cast<uint8_t>(ExceptionCode::IllegalDataValue));
}

TEST_F(DataTest, WriteMultipleRegisters)
{
    // FC16, address 1, quantity 2, byte count 4, values 0x1111 0x2222
    const std::array<uint8_t, 10> request{0x10, 0x00, 0x01, 0x00, 0x02,
                                          0x04, 0x11, 0x11, 0x22, 0x22};

    ASSERT_EQ(process(request), 5U);
    EXPECT_EQ(response[0], 0x10);
    EXPECT_EQ(holding_registers[1], 0x1111U);
    EXPECT_EQ(holding_registers[2], 0x2222U);
    EXPECT_EQ(holding_registers[3], 0x1003U);  // untouched
}

TEST_F(DataTest, WriteMultipleRegistersPastEnd)
{
    // address 7, quantity 2: 7 + 2 > 8
    const std::array<uint8_t, 10> request{0x10, 0x00, 0x07, 0x00, 0x02,
                                          0x04, 0x00, 0x00, 0x00, 0x00};

    ASSERT_EQ(process(request), 2U);
    EXPECT_EQ(response[1],
              static_cast<uint8_t>(ExceptionCode::IllegalDataAddress));
    EXPECT_TRUE(hook_calls.empty());
}

TEST_F(DataTest, ResponseTooSmallIsServerDeviceFailure)
{
    const std::array<uint8_t, 5> request{0x03, 0x00, 0x00, 0x00, 0x04};
    std::array<uint8_t, 4> tiny{};  // needs 2 + 8

    ASSERT_EQ(server.process(request, tiny), 2U);
    EXPECT_EQ(tiny[1],
              static_cast<uint8_t>(ExceptionCode::ServerDeviceFailure));
}

// Mirrors ResponseTooSmallIsServerDeviceFailure but for a write: the model
// is still mutated (a write that already happened cannot be undone just
// because the reply doesn't fit), but the caller must be told it failed to
// receive an acknowledgment via ServerDeviceFailure rather than silence,
// same as the read path.
TEST_F(DataTest, WriteSingleCoilResponseTooSmallIsServerDeviceFailure)
{
    const std::array<uint8_t, 5> request{0x05, 0x00, 0x04, 0xFF, 0x00};
    std::array<uint8_t, 4> tiny{};  // needs kAddressQuantityRequestSize == 5

    ASSERT_EQ(server.process(request, tiny), 2U);
    EXPECT_EQ(tiny[1],
              static_cast<uint8_t>(ExceptionCode::ServerDeviceFailure));
    EXPECT_EQ(coils[4], 1U);
}

}  // namespace
