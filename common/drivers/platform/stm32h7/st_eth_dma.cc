#include "st_eth_dma.h"

#include "reg_helpers.h"
#include "stm32h723xx.h"

namespace EoT::StmH7
{

StEthDma::StEthDma(StEthDmaSettings& config) : params(config)
{
}

bool StEthDma::init()
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
    params.base_addr->DMACTDLAR = reinterpret_cast<uint32_t>(params.tx_head);
    params.base_addr->DMACRDLAR = reinterpret_cast<uint32_t>(params.rx_head);

    // Set tail pointers for descriptors
    params.base_addr->DMACTDTPR = reinterpret_cast<uint32_t>(params.tx_tail);
    params.base_addr->DMACRDTPR = reinterpret_cast<uint32_t>(params.rx_tail);

    if (params.options)
    {
        // ETH_DMACCR - Channel Control Register
        if (params.options->pblx8_mode)
            params.base_addr->DMACCR |= ETH_DMACCR_8PBL;

        SetReg(&(params.base_addr->DMACCR),
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

        SetReg(&(params.base_addr->DMACRCR),
               static_cast<uint32_t>(params.options->rx_buff_size),
               ETH_DMACRCR_RBSZ_Pos, 14);
    }

    // Enable interrupts
    params.base_addr->DMACIER |= ETH_DMACIER_RIE | ETH_DMACIER_TIE;

    // Start TX and RX DMAs
    params.base_addr->DMACTCR |= ETH_DMACTCR_ST_Msk;
    params.base_addr->DMACRCR |= ETH_DMACRCR_SR_Msk;

    return result;
}

}  // namespace EoT::StmH7