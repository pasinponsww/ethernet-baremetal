/**
* @file main.cc
* @brief STM32H723 Ethernet MTL register-level bring-up test.
*
* Exercises the StEthMtl register interface (eth_mtl.h / st_eth_mtl.h)
* directly -- Tx/Rx FIFO store-and-forward vs. cut-through, error/undersized
* frame forwarding, flow-control watermarks, Tx flush, and queue occupancy
* -- and reports each result over USART3 (115200 8N1).
*
* @note This only proves the MTLTQOMR/MTLRQOMR/MTLTQDR/MTLRQDR register
* plumbing is wired correctly. No DMA descriptor rings exist in this repo
* yet, so no frame is actually transmitted or received here -- the packet
* counters are expected to read 0 throughout.
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

void report_u32(HwBoard& hw, const char* name, uint32_t value)
{
    hw.usart.send(bytes("\r\n"));
    hw.usart.send(
        std::span(reinterpret_cast<const uint8_t*>(name), strlen(name)));
    hw.usart.send(bytes(" = "));

    // No printf in this build, so hand-roll a decimal dump.
    char digits[10];
    int n = 0;
    do
    {
        digits[n++] = static_cast<char>('0' + (value % 10U));
        value /= 10U;
    } while (value != 0U && n < static_cast<int>(sizeof(digits)));

    std::array<uint8_t, sizeof(digits)> out{};
    for (int i = 0; i < n; ++i)
        out[i] = static_cast<uint8_t>(digits[n - 1 - i]);
    hw.usart.send(std::span(out.data(), static_cast<size_t>(n)));

    Utils::delay_ms(20);
}

}  // namespace

int main()
{
    board_init();

    auto& hw = get_hw();
    auto& mac = hw.mac;
    auto& mtl = hw.mtl;

    hw.usart.send(bytes("\r\n===== STM32H723 MTL register test =====\r\n"));
    hw.usart.send(
        bytes("(register-level only -- no DMA, no frames actually sent)\r\n"));

    // The MTL sits below the MAC in the same ETH peripheral -- bring the
    // MAC up first so this isn't testing the MTL in isolation from a state
    // no real bring-up sequence would leave it in.
    report(hw, "mac.init()", mac.init());

    report(hw, "mtl.init()", mtl.init());

    // Tx queue: store-and-forward on, then switch to a cut-through
    // threshold. Neither has a dedicated getter, so we can only confirm
    // the write path succeeds.
    report(hw, "configure_tx_queue(store_and_forward=true)",
           mtl.configure_tx_queue(TxQueueConfig{true, TxThreshold::Bits64}));
    report(hw, "configure_tx_queue(cut_through, 128 bits)",
           mtl.configure_tx_queue(TxQueueConfig{false, TxThreshold::Bits128}));

    // Rx queue: store-and-forward, cut-through, and error/undersized
    // packet forwarding flags.
    report(hw, "configure_rx_queue(store_and_forward=true)",
           mtl.configure_rx_queue(
               RxQueueConfig{true, RxThreshold::Bits64, false, false}));
    report(hw, "configure_rx_queue(cut_through, forward errors+undersized)",
           mtl.configure_rx_queue(
               RxQueueConfig{false, RxThreshold::Bits96, true, true}));

    // Flow control thresholds: RFA/RFD are 3-bit fields, so anything above
    // 7 must be rejected by the driver rather than silently truncated.
    report(
        hw, "set_flow_control_thresholds(activate=4, deactivate=2) accepted",
        mtl.set_flow_control_thresholds(FlowControlThresholds{true, 4U, 2U}));
    report(
        hw, "set_flow_control_thresholds(activate=8) rejected",
        !mtl.set_flow_control_thresholds(FlowControlThresholds{true, 8U, 0U}));

    // Tx flush: request it and see whether it's still pending by the time
    // we poll -- FTQ self-clears once the (empty) FIFO finishes flushing,
    // so on real hardware this is expected to read back false almost
    // immediately.
    report(hw, "flush_tx_queue()", mtl.flush_tx_queue());
    report(hw, "is_tx_queue_flush_pending() == false shortly after flush",
           !mtl.is_tx_queue_flush_pending());

    // Queue occupancy: with no DMA rings driving traffic, both FIFOs
    // should read back empty.
    report_u32(hw, "get_tx_packet_count()", mtl.get_tx_packet_count());
    report_u32(hw, "get_rx_packet_count()", mtl.get_rx_packet_count());
    report(hw, "get_tx_packet_count() == 0", mtl.get_tx_packet_count() == 0U);
    report(hw, "get_rx_packet_count() == 0", mtl.get_rx_packet_count() == 0U);

    hw.usart.send(bytes("\r\n===== test done =====\r\n"));

    while (true)
    {
        Utils::delay_ms(1000);
    }

    /**
    * @note Expected results on real hardware:
    * - All PASS lines above.
    * - This test does not exercise actual frame TX/RX; it only proves the
    *   MTL register control surface (StEthMtl) behaves as documented.
    */
}
