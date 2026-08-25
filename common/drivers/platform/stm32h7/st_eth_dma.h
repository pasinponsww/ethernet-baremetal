/**
 * @file dma.h
 * @author Farhaan Khan
 * @date 
 */

#pragma once
#include <eth_dma.h>
#include <concepts>
<<<<<<< HEAD
#include <cstddef>
#include "st_eth_dma_descriptor.h"
#include "stm32h723xx.h"
    == == ==
    =
#include <cstdint>
        >>>>>>> 08f66c3ee3421667e0354b867053b61a57a15e80

    namespace EoT::StmH7
{

<<<<<<< HEAD
    enum class DmaBurstLength : uint8_t
    {
        beats1 = 1, beats2 = 2, beats4 = 4, beats8 = 8,
        == == == =
                     /** 
* Ethernet DMA Descriptor- must be word aligned
* Descriptors generally consist of 4 registers
*/
        struct alignas(32) DmaDescriptor{uint32_t des0; uint32_t des1;
                                         uint32_t des2; uint32_t des3;};

    /** 
* @brief Wrapper around regular descriptor for TX
* @param descriptor.des0 Header or Buffer 1 Addr
* @param descriptor.des1 Buffer 2 Addr or Buffer 1 Addr Extended
* @param descriptor.des2 Control Bits and Buffer/Header Length 
* @param descriptor.des3 More Control Bits and Frame/Payload Length
*/
    class TxDmaDescriptor
    {
        DmaDescriptor descriptor;

        void set_owned_by_dma();

        bool is_owned_by_cpu();

        bool set_buffer();
    };

    /** 
* @brief Wrapper around regular descriptor for RX
* @param descriptor.des0 Buffer 1 Addr
* @param descriptor.des1 Reserved
* @param descriptor.des2 Buffer 2 Addr
* @param descriptor.des3 Some control bits
*/
    class RxDmaDescriptor
    {
        DmaDescriptor descriptor;
    };

    enum class DmaBurstLength : uint8_t
    {
        beats1 = 1,
        beats2 = 2,
        beats4 = 4,
        beats8 = 8,
>>>>>>> 08f66c3ee3421667e0354b867053b61a57a15e80
        beats16 = 16,
        beats32 = 32
    };

<<<<<<< HEAD
    == == == =

>>>>>>> 08f66c3ee3421667e0354b867053b61a57a15e80
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
<<<<<<< HEAD
        uint16_t rx_buff_size{0};             // Must be multiple of 4
        == == == = uint16_t rx_buff_size{0};  // Must be multiple of 4
>>>>>>> 08f66c3ee3421667e0354b867053b61a57a15e80
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
<<<<<<< HEAD
        == == == =

>>>>>>> 08f66c3ee3421667e0354b867053b61a57a15e80
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