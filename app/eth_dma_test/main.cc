/**
* @file main.cc
* @brief STM32H723 Ethernet DMA register-level bring-up test.
*
* Exercises the StEthDma register interface (eth_dma.h / st_eth_dma.h)
* and reports each result over USART3 (115200 8N1).
*
* @note These functions set up descriptors in SRAM and manipulate 
* the RX/TX tail pointer registers to initiate transfers. This test 
* doesn't currently integrate DMA with MAC and MTL yet.
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

    // Set up tx descriptors
    std::array<uint8_t, 10> message{"downers\n"};
    TxDescriptorConfig tx_msg_config{
        .buff1_addr = reinterpret_cast<uint32_t>((message).data()),
        .buff1_len = 10,
        .buff2_addr = 0,
        .buff2_len = 0,
        .is_start_of_packet = true,
        .is_end_of_packet = true
    };
    
    report(hw, "insert tx descriptor", eth_dma.insert_tx_desc(tx_msg_config));


    // Set up rx descriptor
    std::array<uint8_t, kRxBuffSize> rx_buff_1;
    std::array<uint8_t, kRxBuffSize> rx_buff_2;

    RxDescriptorConfig rx_msg_config{
        .buff1_addr = reinterpret_cast<uint32_t>(rx_buff_1.data()),
        .buff2_addr = reinterpret_cast<uint32_t>(rx_buff_2.data())
    };

    report(hw, "insert rx descriptor", eth_dma.insert_rx_desc(rx_msg_config));

    
    // Send packet
    report(hw, "send packet", eth_dma.send_packet());

    // Receive data into one descriptor
    report(hw, "receive", eth_dma.receive(1));

}