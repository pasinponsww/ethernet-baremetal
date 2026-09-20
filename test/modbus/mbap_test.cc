/**
 * @file mbap_test.cc
 * @brief MBAP header codec: field layout, length base point, rejection
 *
 * @author Bex Saw
 */

#include <array>
#include <cstdint>
#include <span>

#include <gtest/gtest.h>

#include "modbus/mbap.h"

namespace
{

using namespace EoT::Net::Modbus;

// transaction 0x1234, protocol 0, length 6 (unit id + 5-byte PDU), unit 0x11
constexpr std::array<uint8_t, 7> kGoodHeader{0x12, 0x34, 0x00, 0x00,
                                             0x00, 0x06, 0x11};

TEST(Mbap, DecodesEveryField)
{
    MbapHeader header{};

    ASSERT_EQ(decode_mbap(kGoodHeader, header), MbapStatus::Ok);
    EXPECT_EQ(header.transaction_id, 0x1234U);
    EXPECT_EQ(header.protocol_id, Spec::kProtocolId);
    EXPECT_EQ(header.length, 6U);
    EXPECT_EQ(header.unit_id, 0x11U);
}

TEST(Mbap, PduSizeExcludesUnitId)
{
    MbapHeader header{};
    header.length = 6U;
    EXPECT_EQ(header.pdu_size(), 5U);

    header.length = 0U;
    EXPECT_EQ(header.pdu_size(), 0U);
}

TEST(Mbap, ShortBufferIsIncomplete)
{
    MbapHeader header{};
    const std::span<const uint8_t> partial{kGoodHeader.data(), 6U};

    EXPECT_EQ(decode_mbap(partial, header), MbapStatus::Incomplete);
}

TEST(Mbap, NonZeroProtocolIdIsRejected)
{
    std::array<uint8_t, 7> bytes{kGoodHeader};
    bytes[3] = 0x01;

    MbapHeader header{};
    EXPECT_EQ(decode_mbap(bytes, header), MbapStatus::BadProtocolId);
}

TEST(Mbap, ZeroLengthIsRejected)
{
    std::array<uint8_t, 7> bytes{kGoodHeader};
    bytes[4] = 0x00;
    bytes[5] = 0x00;

    MbapHeader header{};
    EXPECT_EQ(decode_mbap(bytes, header), MbapStatus::BadLength);
}

TEST(Mbap, LengthOverMaxPduIsRejected)
{
    std::array<uint8_t, 7> bytes{kGoodHeader};

    // length = kMaxPduSize + 2 -> pdu_size = kMaxPduSize + 1
    write_be16(&bytes[4], static_cast<uint16_t>(Spec::kMaxPduSize + 2U));

    MbapHeader header{};
    EXPECT_EQ(decode_mbap(bytes, header), MbapStatus::BadLength);
}

TEST(Mbap, EncodeRecomputesLengthFromPduSize)
{
    MbapHeader header{};
    header.transaction_id = 0xBEEF;
    header.unit_id = 0x2A;
    header.length = 0xFFFF;  // must be ignored

    std::array<uint8_t, Spec::kMbapHeaderSize> out{};
    ASSERT_TRUE(encode_mbap(header, 3U, out));

    const std::array<uint8_t, 7> expected{0xBE, 0xEF, 0x00, 0x00,
                                          0x00, 0x04, 0x2A};
    EXPECT_EQ(out, expected);
}

TEST(Mbap, EncodeRefusesSmallBufferAndBadPduSize)
{
    MbapHeader header{};
    std::array<uint8_t, Spec::kMbapHeaderSize> out{};
    std::array<uint8_t, Spec::kMbapHeaderSize - 1U> small{};

    EXPECT_FALSE(encode_mbap(header, 3U, small));
    EXPECT_FALSE(encode_mbap(header, 0U, out));
    EXPECT_FALSE(encode_mbap(header, Spec::kMaxPduSize + 1U, out));
}

TEST(Mbap, RoundTrip)
{
    MbapHeader in{};
    in.transaction_id = 0x0102;
    in.unit_id = 0x03;

    std::array<uint8_t, Spec::kMbapHeaderSize> wire{};
    ASSERT_TRUE(encode_mbap(in, 10U, wire));

    MbapHeader out{};
    ASSERT_EQ(decode_mbap(wire, out), MbapStatus::Ok);
    EXPECT_EQ(out.transaction_id, in.transaction_id);
    EXPECT_EQ(out.unit_id, in.unit_id);
    EXPECT_EQ(out.pdu_size(), 10U);
}

}  // namespace
