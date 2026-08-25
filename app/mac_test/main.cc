/**
* @file main.cc
* @brief STM32H723 Ethernet MAC register-level bring-up test.
*
* Exercises the StEthMac register interface (eth_mac.h / st_eth_mac.h)
* directly -- init, loopback, TX/RX enable, link settings, and frame-size
* bounds checking -- and reports each result over USART3 (115200 8N1).
*
* @note This only proves the MACCR/MACPFR/MACTFCR/MACRFCR/MACECR register
* plumbing is wired correctly. No DMA descriptor rings exist in this repo
* yet, so no frame is actually transmitted or received here -- that
* requires a separate DMA driver.
*/

#include <array>
#include <cstddef>
#include <cstring>
#include <span>

#include "board.h"
#include "delay.h"

using namespace EoT;

// This test is responsible for testing the MAC process
// ST_MAC_ETHERNET dealing with the MAC registers, but not the DMA or frame processing.

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
    auto& mac = hw.mac;

    hw.usart.send(bytes("\r\n===== STM32H723 MAC register test =====\r\n"));
    hw.usart.send(
        bytes("(register-level only -- no DMA, no frames actually sent)\r\n"));

    report(hw, "init", mac.init());

    // Link settings: no readback register exposed, so we only check the
    // setter itself reports success.
    report(hw, "apply_link_settings(100M, Full)",
           mac.apply_link_settings(PhySpeed::Speed100M, PhyDuplex::Full));
    report(hw, "apply_link_settings(10M, Half)",
           mac.apply_link_settings(PhySpeed::Speed10M, PhyDuplex::Half));

    // Loopback: MACCR.LM is read back through is_loopback_enabled().
    report(hw, "set_loopback(true)", mac.set_loopback(true));
    report(hw, "is_loopback_enabled() == true", mac.is_loopback_enabled());
    report(hw, "set_loopback(false)", mac.set_loopback(false));
    report(hw, "is_loopback_enabled() == false", !mac.is_loopback_enabled());

    // TX/RX enable: MACCR.TE/RE are read back directly from hardware.
    report(hw, "enable_tx(true)", mac.enable_tx(true));
    report(hw, "is_transmitter_enabled() == true",
           mac.is_transmitter_enabled());
    report(hw, "enable_rx(true)", mac.enable_rx(true));
    report(hw, "is_receiver_enabled() == true", mac.is_receiver_enabled());
    report(hw, "enable_tx(false)", mac.enable_tx(false));
    report(hw, "is_transmitter_enabled() == false",
           !mac.is_transmitter_enabled());
    report(hw, "enable_rx(false)", mac.enable_rx(false));
    report(hw, "is_receiver_enabled() == false", !mac.is_receiver_enabled());

    // Receive filter / flow control / MAC address: setters have no
    // dedicated getters, so we can only confirm the write path succeeds.
    ReceiveFilterConfig filter{true, true, false};
    report(hw, "set_receive_filter", mac.set_receive_filter(filter));

    FlowControlConfig flow{true, true, 0x1234U};
    report(hw, "set_flow_control", mac.set_flow_control(flow));

    MacAddress addr{0x02, 0x00, 0x00, 0x00, 0x00, 0x02};
    report(hw, "set_mac_address", mac.set_mac_address(addr));

    // Max frame size: GPSL is a 14-bit field, so anything above 0x3FFF must
    // be rejected by the driver rather than silently truncated.
    report(hw, "set_max_frame_size(1518) accepted",
           mac.set_max_frame_size(1518U));
    report(hw, "set_max_frame_size(0x4000) rejected",
           !mac.set_max_frame_size(0x4000U));

    hw.usart.send(bytes("\r\n===== test done =====\r\n"));

    while (true)
    {
        Utils::delay_ms(1000);
    }

    /**
    * @note Expected results on real hardware:
    * - All PASS lines above.
    * - This test does not exercise actual frame TX/RX; it only proves the
    *   MAC register control surface (StEthMac) behaves as documented.
    */
}
