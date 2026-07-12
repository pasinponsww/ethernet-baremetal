#include <array>
#include <cstdint>
#include <span>

#include "board.h"
#include "delay.h"
#include "eth_mdio.h"

using namespace EoT;

template <size_t N>
constexpr auto bytes(const char (&str)[N])
{
    std::array<uint8_t, N - 1> out{};
    for (size_t i = 0; i < N - 1; ++i) out[i] = static_cast<uint8_t>(str[i]);
    return out;
}

int main()
{
    board_init();

    auto& hw = get_hw();
    auto& mdio = hw.eth_mdio;

    hw.usart.send(bytes("\r\nMDIO test start\r\n"));

    for (uint8_t phy = 0; phy < 32; phy++)
    {
        uint16_t id1 = 0;
        uint16_t id2 = 0;

        auto s1 = mdio.read(phy, 2, id1);
        auto s2 = mdio.read(phy, 3, id2);

        if (s1 == EthMdioStatus::Ok && s2 == EthMdioStatus::Ok &&
            id1 != 0x0000 && id1 != 0xFFFF && id2 != 0x0000 && id2 != 0xFFFF)
        {
            hw.usart.send(bytes("PHY found\r\n"));
            break;
        }
    }

    hw.usart.send(bytes("MDIO test done\r\n"));

    while (true)
    {
        Utils::delay_ms(1000);
    }
}