#include <cstdlib>
#include <span>
#include <vector>
#include "board.h"
#include "crc.h"
#include "delay.h"

using namespace EoT;

// Test data for CRC-32/BZIP2
constexpr uint32_t kActualCrc_1 = 0x86C8C832;
constexpr uint32_t kActualCrc_2 = 0x8F06E7CE;

std::array<uint8_t, 8> txb8{"6742067"};
std::array<uint8_t, 16> txb16{"6742067 6742067"};
std::array<uint8_t, 32> txb32{"6742067 6742067 6742067 6742067"};

std::vector<std::span<uint8_t>> packets = {std::span(txb8), std::span(txb16),
                                           std::span(txb32)};

int main(int argc, char* argv[])
{
    board_init();
    HwBoard& hw = get_hw();

    // Input data
    const uint8_t data8_1[] = {0x01, 0x02, 0x03, 0x04};
    const uint16_t data16_1[] = {0x0102, 0x0304};
    const uint32_t data32_1[] = {0x01020304};

    const uint8_t data8_2[] = {0x0A, 0x0B, 0x0C, 0x0D};
    const uint16_t data16_2[] = {0x0A0B, 0x0C0D};
    const uint32_t data32_2[] = {0x0A0B0C0D};

    /* uint8_t test */

    hw.crc.compare(std::span(data8_1), kActualCrc_1);
    // uart sending data8_1
    Utils::delay_ms(1000);

    hw.crc.compare(std::span(data8_2), kActualCrc_2);
    // uart sending data8_2
    Utils::delay_ms(1000);

    /* uint16_t test */

    hw.crc.compare(std::span(data16_1), kActualCrc_1);
    Utils::delay_ms(1000);

    hw.crc.compare(std::span(data16_2), kActualCrc_2);
    Utils::delay_ms(1000);

    /* uint32_t test */

    hw.crc.compare(std::span(data32_1), kActualCrc_1);
    Utils::delay_ms(1000);

    hw.crc.compare(std::span(data32_2), kActualCrc_2);
    Utils::delay_ms(1000);
    while (1)
    {
        // Loop to test the CRC computation and UART transmission with simulated noise
        for (auto packet : packets)
        {

            uint32_t crc_result;
            hw.crc.compute(packet, crc_result);

            auto string = simulate_noise(packet, 0.01, 0.01);

            // uart send string

            // Delay
            Utils::delay_ms(1000);

            std::array<uint8_t, 20> resultb{"\r\nCrc result: "};

            // uart send resultb

            // Delay
            Utils::delay_ms(1000);

            // Send the CRC result as a string (make decision between 'o' and 'x' based on comparison)
            std::array<uint8_t, 1> crcb{hw.crc.compare(string, crc_result) ? 'o'
                                                                           : 'x'

            };

            // uart send crcb

            // Delay
            Utils::delay_ms(1000);

            std::array<uint8_t, 4> endb{"\r\n"};
            // uart sending endb

            // Delay
            Utils::delay_ms(1000);

            (void)resultb;
            (void)crcb;
            (void)endb;
        }
    }

    return 0;
}