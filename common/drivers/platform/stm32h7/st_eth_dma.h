/**
 * @file dma.h
 * @author Farhaan Khan
 * @date 
 */

#pragma once
#include <eth_dma.h>
#include <concepts>
#include <cstddef>
#include "st_eth_dma_descriptor.h"
#include "stm32h723xx.h"

namespace EoT::StmH7
{

enum class DmaBurstLength : uint8_t
{
    beats1 = 1,
    beats2 = 2,
    beats4 = 4,
    beats8 = 8,
    beats16 = 16,
    beats32 = 32
};

struct StEthDmaCtrlSettings
{
    //ETH_DMASBMR Reg
    bool fixed_burst_length{false};
    bool mixed_burst{false};
    bool address_aligned_beats{false};
    bool rebuild_incrx_burst{false};

    //ETH_DMACCR Reg
    uint8_t descriptor_skip_length{0};
    bool pblx8_mode{false};
    uint16_t maximum_segment_size{0};

    //ETH_DMACTXCR Reg
    DmaBurstLength tx_burst_length{DmaBurstLength::beats1};
    bool tcp_seg_en{false};
    bool osp{false};

    //ETH_DMACRXCR Reg
    DmaBurstLength rx_burst_length{DmaBurstLength::beats1};
    uint16_t rx_buff_size{0};  // Must be multiple of 4
};

struct StEthDmaSettings
{
    RxDmaDescriptor* rx_head{nullptr};
    RxDmaDescriptor* rx_tail{nullptr};
    TxDmaDescriptor* tx_head{nullptr};
    TxDmaDescriptor* tx_tail{nullptr};
    uint16_t rx_length{0};
    uint16_t tx_length{0};
    ETH_TypeDef* base_addr{nullptr};
    StEthDmaCtrlSettings* options{nullptr};
};

class StEthDma : public EoT::EthDma<StEthDma>
{
public:
    StEthDma(StEthDmaSettings& config);
    /** 
    * @brief Initialize Ethernet DMA
    */
    bool init();

private:
    StEthDmaSettings& params;
};

}  // namespace EoT::StmH7