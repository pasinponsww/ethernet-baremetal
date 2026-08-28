/**
* @file main.cc
* @brief STM32H723 Ethernet DMA register-level bring-up test.
*
* Exercises the StEthDma register interface (eth_dma.h / st_eth_dma.h)
* and reports each result over USART3 (115200 8N1).
*
* 
*/

#include <array>
#include <cstddef>
#include <cstring>
#include <span>

#include "board.h"
#include "delay.h"

using namespace EoT;

// This test is responsible for testing the Ethernet DMA process

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
    auto& eth_dma = hw.eth_dma;

    hw.usart.send(bytes("\r\n===== STM32H723 ETH DMA register test =====\r\n"));

    report(hw, "init", eth_dma.init());

    // Set up descriptors
    

    report(hw, "send packet", eth_dma.send_packet());

    report(hw, "receive", eth_dma.receive(3));

}