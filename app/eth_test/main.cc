/**
* @file main.cc
* @brief STM32H723 Ethernet composition bring-up sketch.
*
* Exercises EthernetImpl<StEthMac, StEthMdio, EthDma, EthMtl> (ethernet.h /
* board.h) -- real MAC + PHY register plumbing, with placeholder DMA/MTL
* instances since neither driver exists for this MCU yet.
*
* @note Not expected to build cleanly until EthDma/EthMtl (board.h) grow
* real init()/transmit()/receive() bodies -- this is a structural sketch of
* the composition, not a verified test like mac_test/mtl_test.
*/

#include <array>
#include <cstddef>
#include <cstring>
#include <span>

#include "board.h"
#include "delay.h"

using namespace EoT;

namespace
{

template <size_t N>
constexpr auto bytes(const char (&str)[N])
{
    std::array<uint8_t, N - 1> out{};
    for (size_t i = 0; i < N - 1; ++i) out[i] = static_cast<uint8_t>(str[i]);
    return out;
}

void report(HwBoard& hw, const char* name, bool pass)
{
    hw.usart.send(bytes("\r\n"));
    hw.usart.send(
        std::span(reinterpret_cast<const uint8_t*>(name), strlen(name)));
    hw.usart.send(pass ? bytes(": PASS") : bytes(": FAIL"));
    Utils::delay_ms(20);
}

}  // namespace

int main()
{
    board_init();

    auto& hw = get_hw();
    auto& eth = hw.eth;

    hw.usart.send(bytes("\r\n===== STM32H723 Ethernet composition test =====\r\n"));
    hw.usart.send(bytes("(MAC+PHY registers are real HW; DMA/MTL are placeholders)\r\n"));

    report(hw, "eth.init()", eth.init());

    MacAddress addr{0x02, 0x00, 0x00, 0x00, 0x00, 0x04};
    report(hw, "eth.set_mac_address()", eth.set_mac_address(addr));

    report(hw, "eth.start()", eth.start());

    // Best-effort -- depends on whether a cable + link partner is present.
    bool link = eth.is_link_up();
    hw.usart.send(bytes("\r\neth.is_link_up() = "));
    if (link)
    {
        hw.usart.send(bytes("true"));
    }
    else
    {
        hw.usart.send(bytes("false"));
    }
    Utils::delay_ms(20);

    report(hw, "eth.stop()", eth.stop());

    hw.usart.send(bytes("\r\n===== test done =====\r\n"));

    while (true)
    {
        Utils::delay_ms(1000);
    }
}
