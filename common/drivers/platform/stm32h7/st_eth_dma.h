/**
 * @file st_eth_dma.h
 * @brief STM32 Ethernet DMA Driver
 * @author Farhaan Khan
 * @date
 */

#pragma once
#include <eth_dma.h>

#include <concepts>
#include <cstddef>
#include <cstdint>

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
    // ETH_DMASBMR Reg
    bool fixed_burst_length{false};
    bool mixed_burst{false};
    bool address_aligned_beats{false};
    bool rebuild_incrx_burst{false};

    // ETH_DMACCR Reg
    uint8_t descriptor_skip_length{0};
    bool pblx8_mode{false};
    uint16_t maximum_segment_size{0};

    // ETH_DMACTXCR Reg
    DmaBurstLength tx_burst_length{DmaBurstLength::beats1};
    bool tcp_seg_en{false};
    bool osp{false};

    // ETH_DMACRXCR Reg
    DmaBurstLength rx_burst_length{DmaBurstLength::beats1};
};

struct StEthDmaSettings
{
    RxDmaDescriptor* rx_head{nullptr};
    RxDmaDescriptor* rx_tail{nullptr};
    TxDmaDescriptor* tx_head{nullptr};
    TxDmaDescriptor* tx_tail{nullptr};
    uint16_t rx_length{0};
    uint16_t tx_length{0};
    uint16_t rx_buff_size{0};  // Must be multiple of 4 (2 LSB are ignored)
    ETH_TypeDef* base_addr{nullptr};
    StEthDmaCtrlSettings* options{nullptr};
};

template <uint16_t TxRingSize, uint16_t RxRingSize>
class StEthDma : public EoT::EthDma<StEthDma<TxRingSize, RxRingSize>>
{
public:
    explicit StEthDma(StEthDmaSettings& config, TxDescriptorManager<TxRingSize>& tx_man,
                      RxDescriptorManager<RxRingSize>& rx_man)
        : params(config), tx_manager(tx_man), rx_manager(rx_man)
    {
    }

    /**
     * @brief Initialize Ethernet DMA
     * @return status of operation
     */
    bool init()
    {
        bool result{true};

        // Reset all MAC internal registers and logic
        params.base_addr->DMAMR |= ETH_DMAMR_SWR;

        // Wait for completion of reset process
        // (bit 0 of DMAMR gets cleared when done resetting)
        while (params.base_addr->DMAMR & ETH_DMAMR_SWR)
        {
        }

        if (params.options)
        {
            // Master performs address-aligned transfers on R/W channels
            if (params.options->address_aligned_beats)
                params.base_addr->DMASBMR |= ETH_DMASBMR_AAL;

            // Fixed burst length: when set, the AHB master will initiate burst
            // transfers of the specified length (INCRx or SINGLE).
            if (params.options->fixed_burst_length)
                params.base_addr->DMASBMR |= ETH_DMASBMR_FB;

            /*
                Mixed Burst - AHB master performs undefined burst transfers
                for burst length of 16 or more. Otherwise, master performs
                fixed burst transfers.
            */
            if (params.options->mixed_burst)
                params.base_addr->DMASBMR |= ETH_DMASBMR_MB;

            /*
                When set high and the AHB master gets SPLIT, RETRY,
                or Early Burst Termination (EBT) response, the AHB master interface
                rebuilds the pending beats of any initiated burst transfer with
                INCRx and SINGLE transfers. By default, the AHB master interface
                rebuilds pending beats of an EBT with an unspecified (INCR) burst.
            */
            if (params.options->rebuild_incrx_burst)
                params.base_addr->DMASBMR |= ETH_DMASBMR_RB;
        }

        // Set TX & RX descriptor ring lengths
        uint32_t tx_ring_len =
            static_cast<uint32_t>((params.tx_length - 1) & 0x03ff);
        uint32_t rx_ring_len =
            static_cast<uint32_t>((params.rx_length - 1) & 0x03ff);

        SetReg(&(params.base_addr->DMACTDRLR), tx_ring_len, 0, 10);
        SetReg(&(params.base_addr->DMACRDRLR), rx_ring_len, 0, 10);

        // Set head pointers for descriptors
        params.tx_head = tx_manager.ring_head();
        params.rx_head = rx_manager.ring_head();

        params.base_addr->DMACTDLAR =
            reinterpret_cast<uint32_t>(params.tx_head);
        params.base_addr->DMACRDLAR =
            reinterpret_cast<uint32_t>(params.rx_head);

        // Set tail pointers for descriptors
        params.base_addr->DMACTDTPR =
            reinterpret_cast<uint32_t>(params.tx_tail);
        params.base_addr->DMACRDTPR =
            reinterpret_cast<uint32_t>(params.rx_tail);

        if (params.options)
        {
            // ETH_DMACCR - Channel Control Register
            if (params.options->pblx8_mode)
                params.base_addr->DMACCR |= ETH_DMACCR_8PBL;

            SetReg(
                &(params.base_addr->DMACCR),
                static_cast<uint32_t>(params.options->descriptor_skip_length),
                ETH_DMACCR_DSL_Pos, 3);

            SetReg(&(params.base_addr->DMACCR),
                   static_cast<uint32_t>(params.options->maximum_segment_size),
                   ETH_DMACCR_MSS_Pos, 14);

            // ETH_DMACTXCR - Channel Transmit Control Register
            if (params.options->tcp_seg_en)
                params.base_addr->DMACTCR |= ETH_DMACTCR_TSE;

            if (params.options->osp)
                params.base_addr->DMACTCR |= ETH_DMACTCR_OSP;

            SetReg(&(params.base_addr->DMACTCR),
                   static_cast<uint32_t>(params.options->tx_burst_length),
                   ETH_DMACTCR_TPBL_Pos, 6);

            // ETH_DMACRCR - Channel Receive Control Register
            SetReg(&(params.base_addr->DMACRCR),
                   static_cast<uint32_t>(params.options->rx_burst_length),
                   ETH_DMACRCR_RPBL_Pos, 6);
        }
        // ETH_DMACRCR - Channel Receive Control Register (buffer size must be multiple of 4, 2 LSB are ignored)
        SetReg(&(params.base_addr->DMACRCR),
               static_cast<uint32_t>(params.rx_buff_size), ETH_DMACRCR_RBSZ_Pos,
               14);

        // Enable interrupts
        params.base_addr->DMACIER |= ETH_DMACIER_RIE | ETH_DMACIER_TIE;

        // Start TX and RX DMAs
        params.base_addr->DMACTCR |= ETH_DMACTCR_ST_Msk;
        params.base_addr->DMACRCR |= ETH_DMACRCR_SR_Msk;

        return result;
    }

    /**
     * @brief Add tx descriptor to tx ring
     * @return status of operation
     */
    bool insert_tx_desc(TxDescriptorConfig& config)
    {
        return tx_manager.insert_desc(config);
    }

    /**
     * @brief Add rx descriptor to rx ring
     * @return status of operation
     */
    bool insert_rx_desc(RxDescriptorConfig& config)
    {
        return rx_manager.insert_desc(config);
    }

    /**
     * @brief Send ethernet packet (or all data in descriptors if none is marked as being the end of a packet)
     * @return status of operation
     */
    bool send_packet()
    {
        params.tx_tail = tx_manager.send_packet();
        if (!params.tx_tail)
            return false;

        // Set tail ptr to start transmission
        params.base_addr->DMACTDTPR =
            reinterpret_cast<uint32_t>(params.tx_tail);
        return true;
    }

    /**
     * @brief Receive ethernet data
     * @param num_descriptors number of descriptors to store data into
     * @return status of operation
     */
    bool receive(uint8_t num_descriptors)
    {
        params.rx_tail = rx_manager.receive(num_descriptors);

        if (!params.rx_tail)
            return false;

        // Set tail ptr to start transmission
        params.base_addr->DMACRDTPR =
            reinterpret_cast<uint32_t>(params.rx_tail);
        return true;
    }

private:
    StEthDmaSettings& params;
    TxDescriptorManager<TxRingSize>& tx_manager;
    RxDescriptorManager<RxRingSize>& rx_manager;
};

}  // namespace EoT::StmH7