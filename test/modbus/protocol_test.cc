/**
 * @file protocol_test.cc
 * @brief Modbus::Protocol against the PacketProtocol contract: stream
 *        framing by MBAP length, unit-id filtering, ADU round trip
 *
 * @author Bex Saw
 */

#include <array>
#include <cstdint>
#include <span>
#include <vector>

#include <gtest/gtest.h>

#include "modbus/protocol.h"
#include "packet_protocol.h"

namespace
{

using namespace EoT::Net;
using namespace EoT::Net::Modbus;

// FC3 read holding registers, address 0, quantity 2, wrapped for unit 0x11
// with transaction id 0x0007.
constexpr std::array<uint8_t, 12> kReadAdu{0x00, 0x07, 0x00, 0x00, 0x00, 0x06,
                                           0x11, 0x03, 0x00, 0x00, 0x00, 0x02};

class ProtocolTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        holding_registers = {0xAAAA, 0xBBBB, 0xCCCC, 0xDDDD};
        model.holding_registers = holding_registers;
        server.set_data_model(model);
    }

    std::array<uint16_t, 4> holding_registers{};
    DataModel model{};
    Server server{model};
    Protocol protocol{server};
    std::array<uint8_t, Protocol::kMaxFrameSize> response{};
};

TEST_F(ProtocolTest, ExportsSpecConstants)
{
    EXPECT_EQ(Protocol::kMaxFrameSize, Spec::kMaxAduSize);
    EXPECT_EQ(Protocol::kDefaultPort, 502U);
}

TEST_F(ProtocolTest, FrameSizeOfCompleteAdu)
{
    uint16_t length{0U};
    EXPECT_EQ(protocol.frame_size(kReadAdu, length), FrameStatus::Complete);
    EXPECT_EQ(length, kReadAdu.size());
}

TEST_F(ProtocolTest, FrameSizeOnlySeesFirstOfPipelinedAdus)
{
    std::vector<uint8_t> two{kReadAdu.begin(), kReadAdu.end()};
    two.insert(two.end(), kReadAdu.begin(), kReadAdu.end());

    uint16_t length{0U};
    EXPECT_EQ(protocol.frame_size(two, length), FrameStatus::Complete);
    EXPECT_EQ(length, kReadAdu.size());
}

TEST_F(ProtocolTest, PartialHeaderIsIncomplete)
{
    uint16_t length{0U};
    const std::span<const uint8_t> partial{kReadAdu.data(), 4U};

    EXPECT_EQ(protocol.frame_size(partial, length), FrameStatus::Incomplete);
}

TEST_F(ProtocolTest, HeaderWithoutBodyIsIncomplete)
{
    uint16_t length{0U};
    const std::span<const uint8_t> header_only{kReadAdu.data(),
                                               Spec::kMbapHeaderSize + 2U};

    EXPECT_EQ(protocol.frame_size(header_only, length),
              FrameStatus::Incomplete);
}

TEST_F(ProtocolTest, WrongProtocolIdIsInvalid)
{
    std::array<uint8_t, 12> bad{kReadAdu};
    bad[2] = 0x12;

    uint16_t length{0U};
    EXPECT_EQ(protocol.frame_size(bad, length), FrameStatus::Invalid);
}

TEST_F(ProtocolTest, HandleRoundTripsMbapAndDispatches)
{
    const uint16_t written{protocol.handle(kReadAdu, response)};

    // 7-byte MBAP + FC + byte count + 2 registers
    ASSERT_EQ(written, Spec::kMbapHeaderSize + 6U);

    // transaction id and unit id echoed, length recomputed for the response
    EXPECT_EQ(response[0], 0x00);
    EXPECT_EQ(response[1], 0x07);
    EXPECT_EQ(response[4], 0x00);
    EXPECT_EQ(response[5], 0x07);  // unit id + 6-byte PDU
    EXPECT_EQ(response[6], 0x11);

    EXPECT_EQ(response[7], 0x03);
    EXPECT_EQ(response[8], 0x04);
    EXPECT_EQ(response[9], 0xAA);
    EXPECT_EQ(response[10], 0xAA);
    EXPECT_EQ(response[11], 0xBB);
    EXPECT_EQ(response[12], 0xBB);

    EXPECT_EQ(protocol.statistics().requests, 1U);
    EXPECT_EQ(server.statistics().requests, 1U);
}

TEST_F(ProtocolTest, ExceptionResponseIsAlsoWrapped)
{
    std::array<uint8_t, 12> adu{kReadAdu};
    adu[7] = 0x2B;  // unsupported function

    const uint16_t written{protocol.handle(adu, response)};

    ASSERT_EQ(written, Spec::kMbapHeaderSize + 2U);
    EXPECT_EQ(response[5], 0x03);  // unit id + 2-byte exception PDU
    EXPECT_EQ(response[7], 0x2B | Spec::kExceptionFlag);
    EXPECT_EQ(response[8],
              static_cast<uint8_t>(ExceptionCode::IllegalFunction));
}

TEST_F(ProtocolTest, DefaultUnitIdAnswersAnyUnit)
{
    for (const uint8_t unit : {0x00, 0x01, 0x11, 0xFF})
    {
        std::array<uint8_t, 12> adu{kReadAdu};
        adu[6] = unit;

        EXPECT_GT(protocol.handle(adu, response), 0U) << "unit " << +unit;
        EXPECT_EQ(response[6], unit);
    }

    EXPECT_EQ(protocol.statistics().filtered, 0U);
}

TEST_F(ProtocolTest, StrictUnitIdFiltersOthers)
{
    Protocol strict{server, ProtocolConfig{.unit_id = 0x11}};

    EXPECT_GT(strict.handle(kReadAdu, response), 0U);

    std::array<uint8_t, 12> other{kReadAdu};
    other[6] = 0x12;

    EXPECT_EQ(strict.handle(other, response), 0U);
    EXPECT_EQ(strict.statistics().filtered, 1U);
    EXPECT_EQ(strict.statistics().requests, 1U);
}

TEST_F(ProtocolTest, EmptyPduIsSilent)
{
    // length 1: unit id only, no PDU
    const std::array<uint8_t, 7> adu{0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x11};

    EXPECT_EQ(protocol.handle(adu, response), 0U);
    EXPECT_EQ(protocol.statistics().silent, 1U);
}

TEST_F(ProtocolTest, HandleRefusesTinyResponseBuffer)
{
    std::array<uint8_t, Spec::kMbapHeaderSize - 1U> tiny{};

    EXPECT_EQ(protocol.handle(kReadAdu, tiny), 0U);
}

}  // namespace
