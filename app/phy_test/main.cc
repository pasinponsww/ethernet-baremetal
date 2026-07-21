/**
* @file main.cc
* @brief LAN8742 PHY bring-up test.
*
* Proves MDIO communication with the PHY by reading back its 32-bit
* identifier and checking it against the expected LAN8742 ID. If the PHY at
* the configured address does not answer, it scans all 32 MDIO addresses so a
* mis-strapped address still shows up. Results are reported over USART3
* (115200 8N1).
*/

#include <array>
#include <cstdint>
#include <span>

#include "board.h"
#include "delay.h"
#include "lan8742.h"

using namespace EoT;

namespace
{

// Send a compile-time string literal (without the trailing NUL).
template <size_t N>
consteval auto bytes(const char (&str)[N])
{
    std::array<uint8_t, N - 1> out{};
    for (size_t i = 0; i < N - 1; ++i) out[i] = static_cast<uint8_t>(str[i]);
    return out;
}

// Send a 32-bit value as "0xXXXXXXXX".
template <typename TUsart>
void send_hex32(Usart<TUsart>& usart, uint32_t value)
{
    static constexpr char kHex[] = "0123456789ABCDEF";
    std::array<uint8_t, 10> buf{'0', 'x'};
    for (uint32_t i = 0; i < 8; ++i)
    {
        buf[2 + i] = static_cast<uint8_t>(kHex[(value >> ((7 - i) * 4)) & 0xF]);
    }
    usart.send(buf);
}

// Send a single byte as "0xXX".
template <typename TUsart>
void send_hex8(Usart<TUsart>& usart, uint8_t value)
{
    static constexpr char kHex[] = "0123456789ABCDEF";
    std::array<uint8_t, 4> buf{'0', 'x'};
    buf[2] = static_cast<uint8_t>(kHex[(value >> 4) & 0xF]);
    buf[3] = static_cast<uint8_t>(kHex[value & 0xF]);
    usart.send(buf);
}

}  // namespace

int main()
{
    board_init();

    auto& hw = get_hw();
    auto& usart = hw.usart;
    auto& mdio = hw.eth_mdio;
    auto& phy = hw.phy;

    usart.send(bytes("\r\n===== LAN8742 PHY test =====\r\n"));

    // 1. Read the PHY ID at the configured address
    uint32_t id = 0;
    bool ok = phy.read_id(id);

    usart.send(bytes("PHY ID: "));
    send_hex32(usart, id);
    usart.send(bytes("\r\n"));

    if (ok && phy.is_valid_id(id))
    {
        usart.send(bytes("Result: LAN8742 detected (ID valid)\r\n"));
    }
    else
    {
        // 2. Fallback: scan every MDIO address for a live PHY
        usart.send(bytes(
            "Configured address did not match, scanning MDIO bus...\r\n"));

        bool found = false;
        for (uint8_t addr = 0; addr < 32; ++addr)
        {
            uint16_t id1 = 0;
            uint16_t id2 = 0;

            auto s1 =
                mdio.read(addr, static_cast<uint8_t>(PhyReg::PHYID1), id1);
            auto s2 =
                mdio.read(addr, static_cast<uint8_t>(PhyReg::PHYID2), id2);

            // A floating bus reads back as all-zeros or all-ones.
            if (s1 == EthMdioStatus::Ok && s2 == EthMdioStatus::Ok &&
                id1 != 0x0000 && id1 != 0xFFFF)
            {
                uint32_t scan_id = (static_cast<uint32_t>(id1) << 16) | id2;

                usart.send(bytes("  PHY at addr "));
                send_hex8(usart, addr);
                usart.send(bytes(" id "));
                send_hex32(usart, scan_id);
                usart.send(bytes("\r\n"));
                found = true;
            }
        }

        if (found)
        {
            usart.send(bytes("Result: PHY(s) found on bus\r\n"));
        }
        else
        {
            usart.send(bytes("Result: no PHY responded\r\n"));
        }
    }

    // 3. Report link state (best-effort)
    if (phy.is_link_up())
    {
        PhySettings link{};
        if (phy.current_link_state(link) == PhyStatus::Ok)
        {
            if (link.speed == PhySpeed::Speed100M)
            {
                usart.send(bytes("Link: up, 100M "));
            }
            else
            {
                usart.send(bytes("Link: up, 10M "));
            }

            if (link.duplex == PhyDuplex::Full)
            {
                usart.send(bytes("full\r\n"));
            }
            else
            {
                usart.send(bytes("half\r\n"));
            }
        }
        else
        {
            usart.send(bytes("Link: up\r\n"));
        }
    }
    else
    {
        usart.send(bytes("Link: down\r\n"));
    }

    usart.send(bytes("===== test done =====\r\n"));

    while (true)
    {
        Utils::delay_ms(1000);
    }

    /**
    * @note Testcase expected results:
    * - PHY ID read back matches LAN8742 ID (0x0007C0F0)
    * - Link state is reported correctly (up/down, speed, duplex)
    * - If the PHY is not at the configured address, it is found during the scan
    *   and its ID is reported.
    * - If no PHY responds, the test reports "no PHY responded".
    * 
    */
}
