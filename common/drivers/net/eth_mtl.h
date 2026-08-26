/**
 * @file eth_mtl.h
 * @brief Generic Ethernet MTL (MAC Transaction Layer) CRTP interface
 * @author Bex Saw
 * @date 8/25/2026
 * @note The MTL sits between the DMA and the MAC core. It owns the Tx/Rx
 *       FIFOs and decides when a frame is handed off (store-and-forward vs.
 *       cut-through threshold) and when flow control should kick in. It does
 *       not touch addressing/framing (that's the MAC) and it does not touch
 *       descriptors/buffers (that's the DMA).
 */

#pragma once
#include <concepts>
#include <cstdint>

namespace EoT
{

/**
 * @brief Cut-through threshold for the Tx FIFO (MTLTQOMR.TTC)
 * @note Only meaningful when TxQueueConfig::store_and_forward is false.
 */
enum class TxThreshold : uint8_t
{
    Bits32 = 0,
    Bits64,
    Bits96,
    Bits128,
    Bits192,
    Bits256,
    Bits384,
    Bits512,
};

/**
 * @brief Cut-through threshold for the Rx FIFO (MTLRQOMR.RTC)
 * @note Only meaningful when RxQueueConfig::store_and_forward is false.
 */
enum class RxThreshold : uint8_t
{
    Bits64 = 0,
    Bits32,
    Bits96,
    Bits128,
};

/**
 * @brief Tx FIFO configuration
 * @param store_and_forward Wait for the whole frame before releasing it to
 *        the MAC (safe, adds latency). If false, release starts once
 *        `threshold` bytes are buffered (lower latency, can underrun and
 *        forward a runt frame on a slow system bus).
 * @param threshold Cut-through release point, ignored when store_and_forward
 *        is true.
 */
struct TxQueueConfig
{
    bool store_and_forward{true};
    TxThreshold threshold{TxThreshold::Bits64};
};

/**
 * @brief Rx FIFO configuration
 * @param store_and_forward Wait for the whole frame (and its CRC/status) to
 *        land before releasing it to the DMA. If false, release starts once
 *        `threshold` bytes are buffered.
 * @param threshold Cut-through release point, ignored when store_and_forward
 *        is true.
 * @param forward_error_packets Forward frames with CRC/length/overflow
 *        errors to the DMA instead of dropping them in the MTL.
 * @param forward_undersized_packets Forward good frames shorter than 64
 *        bytes instead of dropping them.
 */
struct RxQueueConfig
{
    bool store_and_forward{true};
    RxThreshold threshold{RxThreshold::Bits64};
    bool forward_error_packets{false};
    bool forward_undersized_packets{false};
};

/**
 * @brief Rx FIFO watermarks that drive IEEE 802.3x pause-frame generation
 * @param enable Let the Rx FIFO fill level automatically trigger the MAC's
 *        flow control (MTLRQOMR.EHFC), on top of whatever the MAC layer's
 *        own FlowControlConfig::tx_pause set.
 * @param activate_threshold Fill level (MTLRQOMR.RFA) above which a pause
 *        frame is requested.
 * @param deactivate_threshold Fill level (MTLRQOMR.RFD) below which the
 *        pause condition clears.
 */
struct FlowControlThresholds
{
    bool enable{false};
    uint8_t activate_threshold{0U};
    uint8_t deactivate_threshold{0U};
};

// clang-format off
template <typename T>
concept MtlReq = requires(
    T& mtl,
    const T& const_mtl,
    const TxQueueConfig& tx_config,
    const RxQueueConfig& rx_config,
    const FlowControlThresholds& flow_control)
{
    { mtl.init() } -> std::same_as<bool>;
    { mtl.configure_tx_queue(tx_config) } -> std::same_as<bool>;
    { mtl.configure_rx_queue(rx_config) } -> std::same_as<bool>;
    { mtl.set_flow_control_thresholds(flow_control) } -> std::same_as<bool>;
    { mtl.flush_tx_queue() } -> std::same_as<bool>;
    { const_mtl.is_tx_queue_flush_pending() } -> std::same_as<bool>;
    { const_mtl.get_tx_packet_count() } -> std::same_as<uint32_t>;
    { const_mtl.get_rx_packet_count() } -> std::same_as<uint32_t>;
};
// clang-format on

template <typename T>
class Mtl
{
public:
    /**
    * @brief constructor which uses a static_assert as normal concept syntax doesn't work
    */
    Mtl()
    {
        static_assert(MtlReq<T> && std::derived_from<T, Mtl<T>>);
    }

    /**
    * @brief Initialize the MTL Tx/Rx FIFOs
    * @return true if initialization was successful, false otherwise
    */
    bool init()
    {
        return self().init();
    }

    /**
    * @brief Configure the Tx FIFO's store-and-forward / cut-through behavior
    * @param config The Tx queue configuration
    * @return true if the operation was successful, false otherwise
    */
    bool configure_tx_queue(const TxQueueConfig& config)
    {
        return self().configure_tx_queue(config);
    }

    /**
    * @brief Configure the Rx FIFO's store-and-forward / cut-through behavior
    *        and error/undersized frame forwarding
    * @param config The Rx queue configuration
    * @return true if the operation was successful, false otherwise
    */
    bool configure_rx_queue(const RxQueueConfig& config)
    {
        return self().configure_rx_queue(config);
    }

    /**
    * @brief Configure the Rx FIFO watermarks used for automatic flow control
    * @param config The flow control threshold configuration
    * @return true if the operation was successful, false otherwise
    */
    bool set_flow_control_thresholds(const FlowControlThresholds& config)
    {
        return self().set_flow_control_thresholds(config);
    }

    /**
    * @brief Flush the Tx FIFO, discarding any buffered frame
    * @return true if the flush was requested successfully, false otherwise
    */
    bool flush_tx_queue()
    {
        return self().flush_tx_queue();
    }

    /**
    * @brief Whether a previously requested Tx queue flush is still pending
    *        (the flush bit is self-clearing once complete)
    */
    bool is_tx_queue_flush_pending() const
    {
        return self().is_tx_queue_flush_pending();
    }

    /**
    * @brief Number of complete packets currently buffered in the Tx FIFO
    */
    uint32_t get_tx_packet_count() const
    {
        return self().get_tx_packet_count();
    }

    /**
    * @brief Number of complete packets currently buffered in the Rx FIFO
    */
    uint32_t get_rx_packet_count() const
    {
        return self().get_rx_packet_count();
    }

private:
    T& self()
    {
        return static_cast<T&>(*this);
    }

    const T& self() const
    {
        return static_cast<const T&>(*this);
    }
};

}  // namespace EoT
