#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <random>
#include <span>
#include <vector>
#include "board.h"
#include "crc.h"
#include "delay.h"

using namespace EoT;

constexpr uint32_t kCrcData1{0x86C8C832};  // CRC-32/BZIP2 of 01 02 03 04
constexpr uint32_t kCrcData2{0x8F06E7CE};  // CRC-32/BZIP2 of 0A 0B 0C 0D

// This auto bytes is using for sending string to the USART, so we can use it to send the name of the test and the result of the test
template <size_t N>
constexpr auto bytes(const char (&str)[N])
{
    std::array<uint8_t, N - 1> out{};
    for (size_t i = 0; i < N - 1; ++i) out[i] = static_cast<uint8_t>(str[i]);
    return out;
}

template <typename T, size_t N>
void run_crc_check(HwBoard& hw, const char* name, const std::array<T, N>& data,
                   uint32_t expected)
{
    const bool pass = hw.crc.compare(std::span(data), expected);

    hw.usart.send(bytes("\r\n"));
    hw.usart.send(
        std::span(reinterpret_cast<const uint8_t*>(name), strlen(name)));
    if (pass)
    {
        hw.usart.send(bytes(": PASS"));
    }
    else
    {
        hw.usart.send(bytes(": FAIL"));
    }

    Utils::delay_ms(100);
}

// Routine is up above =============================================================

int main(int argc, char* argv[])
{
    board_init();
    HwBoard& hw = get_hw();

    const std::array<uint8_t, 4> data8_1{0x01, 0x02, 0x03, 0x04};
    const std::array<uint8_t, 4> data8_2{0x0A, 0x0B, 0x0C, 0x0D};

    const std::array<uint16_t, 2> data16_1{0x0102, 0x0304};
    const std::array<uint16_t, 2> data16_2{0x0A0B, 0x0C0D};

    const std::array<uint32_t, 1> data32_1{0x01020304};
    const std::array<uint32_t, 1> data32_2{0x0A0B0C0D};

    run_crc_check(hw, "uint8 data 1", data8_1, kCrcData1);
    run_crc_check(hw, "uint8 data 2", data8_2, kCrcData2);

    run_crc_check(hw, "uint16 data 1", data16_1, kCrcData1);
    run_crc_check(hw, "uint16 data 2", data16_2, kCrcData2);

    run_crc_check(hw, "uint32 data 1", data32_1, kCrcData1);
    run_crc_check(hw, "uint32 data 2", data32_2, kCrcData2);

    auto packet8 = bytes("6742067");
    auto packet16 = bytes("6742067 6742067");
    auto packet32 = bytes("6742067 6742067 6742067 6742067");

    std::array<std::span<uint8_t>, 3> packets{
        std::span(packet8),
        std::span(packet16),
        std::span(packet32),
    };

    while (true)
    {
        for (auto packet : packets)
        {
            uint32_t original_crc = 0;
            hw.crc.compute(packet, original_crc);

            auto noisy = simulate_noise(packet, 0.0, 0.0);
            std::span<const uint8_t> noisy_span(noisy.data(), noisy.size());

            bool unchanged =
                noisy.size() == packet.size() &&
                std::equal(noisy.begin(), noisy.end(), packet.begin());

            bool crc_ok = hw.crc.compare(noisy_span, original_crc);

            if (unchanged)
            {
                hw.usart.send(bytes("\r\nNo noise: CRC OK expected"));
            }
            else if (!crc_ok)
            {
                hw.usart.send(bytes("\r\nNoise detected: CRC FAIL expected"));
            }
            else
            {
                hw.usart.send(
                    bytes("\r\nERROR: noise happened but CRC still OK"));
            }

            Utils::delay_ms(100);
        }
    }
}