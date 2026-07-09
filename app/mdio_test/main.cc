#include <array>
#include <cstdint>
#include <cstring>
#include <span>
#include "board.h"
#include "delay.h"

using namespace EoT;

constexpr uint8_t hex_digit(uint8_t nibble)
{
    return static_cast<uint8_t>(nibble < 10U ? ('0' + nibble)
                                             : ('A' + (nibble - 10U)));
}

/**
* @note The reason we having this test is because the ST HAL driver for MDIO is not working properly. 
* It does not allow us to read/write to the PHY registers directly, and it does not provide a way to check for errors. 
* This test is a simple way to verify that we can read/write to the PHY registers directly using the MDIO interface.
* It seperate the DINR & DOUTR registers for each PHY register, so we can read/write to the PHY registers directly without having to use the ST HAL driver.
*/
std::array<uint8_t, 6> hex16(uint16_t value)
{
    return {static_cast<uint8_t>('0'),
            static_cast<uint8_t>('x'),
            hex_digit(static_cast<uint8_t>((value >> 12U) & 0xFU)),
            hex_digit(static_cast<uint8_t>((value >> 8U) & 0xFU)),
            hex_digit(static_cast<uint8_t>((value >> 4U) & 0xFU)),
            hex_digit(static_cast<uint8_t>(value & 0xFU))};
}

template <typename TUsart>
void send_string(TUsart& usart, const char* text)
{
    usart.send(std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(text),
                                        std::strlen(text)));
}

void run_mdio_test(HwBoard& hw, uint8_t phy, uint8_t reg, uint16_t pattern)
{
    uint16_t readback{0U};

    const auto w_stat = hw.mdio.write(phy, reg, pattern);
    const auto r_stat = hw.mdio.read(phy, reg, readback);

    const auto read_hex = hex16(readback);

    send_string(hw.usart, "\r\nMDIO write=");
    send_string(hw.usart, w_stat == MdioStatus::OK ? "OK" : "FAIL");
    send_string(hw.usart, " read=");
    send_string(hw.usart, r_stat == MdioStatus::OK ? "OK" : "FAIL");
    send_string(hw.usart, " readback=");
    hw.usart.send(std::span<const uint8_t>(read_hex.data(), read_hex.size()));
    send_string(hw.usart, "\r\n");
}

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    constexpr uint8_t kPhyAddr{0U};

    board_init();
    HwBoard& hw = get_hw();

    send_string(hw.usart, "\r\nTest Start\r\n");

    while (true)
    {
        uint16_t id1{0U};
        uint16_t id2{0U};
        uint16_t bsr{0U};

        const auto id1_stat = hw.mdio.read(kPhyAddr, 2U, id1);
        const auto id2_stat = hw.mdio.read(kPhyAddr, 3U, id2);
        const auto bsr_stat = hw.mdio.read(kPhyAddr, 1U, bsr);

        const auto id1_hex = hex16(id1);
        const auto id2_hex = hex16(id2);
        const auto bsr_hex = hex16(bsr);

        send_string(hw.usart, "\r\nPHY ID1 read=");
        send_string(hw.usart, id1_stat == MdioStatus::OK ? "OK" : "FAIL");
        send_string(hw.usart, " value=");
        hw.usart.send(std::span<const uint8_t>(id1_hex.data(), id1_hex.size()));

        send_string(hw.usart, "\r\nPHY ID2 read=");
        send_string(hw.usart, id2_stat == MdioStatus::OK ? "OK" : "FAIL");
        send_string(hw.usart, " value=");
        hw.usart.send(std::span<const uint8_t>(id2_hex.data(), id2_hex.size()));

        send_string(hw.usart, "\r\nBSR read=");
        send_string(hw.usart, bsr_stat == MdioStatus::OK ? "OK" : "FAIL");
        send_string(hw.usart, " value=");
        hw.usart.send(std::span<const uint8_t>(bsr_hex.data(), bsr_hex.size()));

        send_string(hw.usart, "\r\n");

        Utils::delay_ms(1000);
    }
}