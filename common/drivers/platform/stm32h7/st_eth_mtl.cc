#include "st_eth_mtl.h"
#include "reg_helpers.h"

namespace EoT::StmH7
{
StEthMtl::StEthMtl(const StEthMtlParams& params)
    : base_addr(params.base_addr), settings(params.settings)
{
}

bool StEthMtl::init()
{
    if (base_addr == nullptr)
    {
        return false;
    }

    // Note: MTLRQOMR.RQS (Rx FIFO size) is left at its reset default. The
    // H723 only implements a single Tx/Rx queue pair, so the entire FIFO is
    // already allocated to queue 0 -- there's no other queue to share it
    // with.

    // Basically in the init process of the MTL comprise of the tx_queue and
    // rx_queue configuration, and the flow control thresholds.
    if (!configure_tx_queue(settings.tx_queue))
    {
        return false;
    }

    if (!configure_rx_queue(settings.rx_queue))
    {
        return false;
    }

    // Later on it sets the flow control thresholds, which is a separate configuration step.
    return set_flow_control_thresholds(settings.flow_control);
}

bool StEthMtl::configure_tx_queue(const TxQueueConfig& config)
{
    if (base_addr == nullptr)
    {
        return false;
    }

    // Transmit Store and Forward (TSF) is in MTLTQOMR. If TSF is disabled,
    // the Transmit Threshold Control (TTC) field in MTLTQOMR decides how
    // many bytes must be in the Tx FIFO before the MTL will start sending a frame.
    //  If TSF is enabled, TTC is ignored and the MTL will always wait for the entire
    // frame to be in the FIFO before sending.
    SetReg(&base_addr->MTLTQOMR, config.store_and_forward ? 1U : 0U,
           ETH_MTLTQOMR_TSF_Pos, 1U);

    if (!config.store_and_forward)
    {
        SetReg(&base_addr->MTLTQOMR, static_cast<uint32_t>(config.threshold),
               ETH_MTLTQOMR_TTC_Pos, 3U);
    }

    settings.tx_queue = config;
    return true;
}

bool StEthMtl::configure_rx_queue(const RxQueueConfig& config)
{
    if (base_addr == nullptr)
    {
        return false;
    }

    // Receive Store and Forward (RSF) is in MTLRQOMR. As with Tx, RTC only
    // matters in cut-through mode.
    SetReg(&base_addr->MTLRQOMR, config.store_and_forward ? 1U : 0U,
           ETH_MTLRQOMR_RSF_Pos, 1U);

    if (!config.store_and_forward)
    {
        SetReg(&base_addr->MTLRQOMR, static_cast<uint32_t>(config.threshold),
               ETH_MTLRQOMR_RTC_Pos, 2U);
    }

    // Forward Error Packets (FEP) and Forward Undersized Good Packets (FUP)
    // -- both in MTLRQOMR -- decide whether the MTL drops bad/runt frames
    // itself or hands them up to the DMA anyway.
    SetReg(&base_addr->MTLRQOMR, config.forward_error_packets ? 1U : 0U,
           ETH_MTLRQOMR_FEP_Pos, 1U);
    SetReg(&base_addr->MTLRQOMR, config.forward_undersized_packets ? 1U : 0U,
           ETH_MTLRQOMR_FUP_Pos, 1U);

    settings.rx_queue = config;
    return true;
}

bool StEthMtl::set_flow_control_thresholds(const FlowControlThresholds& config)
{
    if (base_addr == nullptr)
    {
        return false;
    }

    // RFA/RFD are 3-bit fields
    if (config.activate_threshold > 0x7U || config.deactivate_threshold > 0x7U)
    {
        return false;
    }

    // Enable Hardware Flow Control (EHFC) lets the Rx FIFO fill level drive
    // the MAC's pause-frame generation; RFA/RFD are the activate/deactivate
    // watermarks it uses to do so. All three live in MTLRQOMR.
    SetReg(&base_addr->MTLRQOMR, config.enable ? 1U : 0U, ETH_MTLRQOMR_EHFC_Pos,
           1U);
    SetReg(&base_addr->MTLRQOMR, config.activate_threshold,
           ETH_MTLRQOMR_RFA_Pos, 3U);
    SetReg(&base_addr->MTLRQOMR, config.deactivate_threshold,
           ETH_MTLRQOMR_RFD_Pos, 3U);

    settings.flow_control = config;
    return true;
}

bool StEthMtl::flush_tx_queue()
{
    if (base_addr == nullptr)
    {
        return false;
    }

    // Flush Transmit Queue (FTQ) in MTLTQOMR is self-clearing once the
    // flush completes.
    SetReg(&base_addr->MTLTQOMR, 1U, ETH_MTLTQOMR_FTQ_Pos, 1U);

    return true;
}

bool StEthMtl::is_tx_queue_flush_pending() const
{
    if (base_addr == nullptr)
    {
        return false;
    }

    // This function just check the status of the FTQ bit in MTLTQOMR. If it's set, the flush is still in progress.
    return (base_addr->MTLTQOMR & ETH_MTLTQOMR_FTQ) != 0U;
}

uint32_t StEthMtl::get_tx_packet_count() const
{
    if (base_addr == nullptr)
    {
        return 0U;
    }

    // This function reads the tx packet count from the MTLTQDR register.
    // The count indicate the number of packets transmitted by the MTL for the configured Tx queue.
    // It's like it's own ring buffer for counting packets, and the hardware increments it automatically.
    return (base_addr->MTLTQDR & ETH_MTLTQDR_PTXQ) >> ETH_MTLTQDR_PTXQ_Pos;
}

uint32_t StEthMtl::get_rx_packet_count() const
{
    if (base_addr == nullptr)
    {
        return 0U;
    }

    // This one is similar process to the tx packet count, but for the rx queue.
    // That reads the number of the packet of the rx queue that have been received by the MTL and handed off to the DMA.
    return (base_addr->MTLRQDR & ETH_MTLRQDR_PRXQ) >> ETH_MTLRQDR_PRXQ_Pos;
}

}  // namespace EoT::StmH7
