/**
*  @file st_eth_mtl.h
*  @brief MTL (MAC Transaction Layer) for STM32H7
*  @author Bex Saw
*  @date 8/25/2026
*/

#pragma once
#include <cstdint>
#include "eth_mtl.h"
#include "stm32h723xx.h"

namespace EoT
{
namespace StmH7
{

/**
* @brief Settings for the STM32H7 MTL layer
* @param tx_queue Tx FIFO store-and-forward / cut-through configuration
* @param rx_queue Rx FIFO store-and-forward / cut-through configuration and
*        error/undersized frame forwarding
* @param flow_control Rx FIFO watermark configuration for automatic flow
*        control (MTLRQOMR.EHFC/RFA/RFD)
*/
struct StEthMtlSettings
{
    TxQueueConfig tx_queue;
    RxQueueConfig rx_queue;
    FlowControlThresholds flow_control;
};

/**
* @brief Parameters for the STM32H7 MTL layer
* @param settings MTL settings
* @param base_addr Base address of the ETH peripheral
*/
struct StEthMtlParams
{
    StEthMtlSettings settings;
    ETH_TypeDef* base_addr;
};

class StEthMtl : public Mtl<StEthMtl>
{
public:
    explicit StEthMtl(const StEthMtlParams& params);

    /**
    * @brief Initialize the MTL Tx/Rx FIFOs from the settings passed at
    *        construction
    * @return true if initialization was successful, false otherwise
    */
    bool init();

    /**
    * @brief Configure the Tx FIFO's store-and-forward / cut-through mode
    *        (MTLTQOMR.TSF/TTC)
    * @param config The Tx queue configuration
    * @return true if the operation was successful, false otherwise
    */
    bool configure_tx_queue(const TxQueueConfig& config);

    /**
    * @brief Configure the Rx FIFO's store-and-forward / cut-through mode
    *        and error/undersized frame forwarding
    *        (MTLRQOMR.RSF/RTC/FEP/FUP)
    * @param config The Rx queue configuration
    * @return true if the operation was successful, false otherwise
    */
    bool configure_rx_queue(const RxQueueConfig& config);

    /**
    * @brief Configure the Rx FIFO watermarks used for automatic flow
    *        control (MTLRQOMR.EHFC/RFA/RFD)
    * @param config The flow control threshold configuration, RFA/RFD are
    *        3-bit fields (0-7)
    * @return true if the operation was successful, false if a threshold
    *        doesn't fit
    */
    bool set_flow_control_thresholds(const FlowControlThresholds& config);

    /**
    * @brief Request a flush of the Tx FIFO (MTLTQOMR.FTQ), discarding any
    *        buffered frame
    * @return true if the flush was requested successfully, false otherwise
    */
    bool flush_tx_queue();

    /**
    * @brief Whether a previously requested Tx queue flush is still pending
    *        (reads MTLTQOMR.FTQ directly, the bit self-clears once the
    *        flush completes)
    */
    bool is_tx_queue_flush_pending() const;

    /**
    * @brief Number of complete packets currently buffered in the Tx FIFO
    *        (reads MTLTQDR.PTXQ)
    */
    uint32_t get_tx_packet_count() const;

    /**
    * @brief Number of complete packets currently buffered in the Rx FIFO
    *        (reads MTLRQDR.PRXQ)
    */
    uint32_t get_rx_packet_count() const;

private:
    ETH_TypeDef* base_addr;
    StEthMtlSettings settings;
};
}  // namespace StmH7
}  // namespace EoT
