#include <array>
#include <cstddef>
#include <cstdint>
#include "reg_helpers.h"

namespace EoT::StmH7
{

// DMA TX descriptor control bits

/*
* There's still some control bit definitions missing- minimal implementation for now.
* Can add support for TCP stuff, timestamp stuff, context descriptor type, 
* write-back descriptor type, etc.
*/

// clang-format off
#define ETH_TDES2_IOC_Pos       (31U)
#define ETH_TDES2_IOC           (0x1UL << ETH_TDES2_IOC_Pos)      /* Interrupt on Completion*/
#define ETH_TDES2_B2L_Pos       (16U)                             /* Buffer 2 Length Starting Bit Position */
#define ETH_TDES2_B2L_Bit_Len   (14U)                         /* B2L Field bit length */
#define ETH_TDES2_B1L_Pos       (0U)                              /* Buffer 1 Length Starting Bit Position */
#define ETH_TDES2_B1L_Bit_Len   (14U)                         /* B1L Field bit length */
#define ETH_TDES3_OWN_Pos       (31U)
#define ETH_TDES3_OWN           (0x1UL << ETH_TDES3_OWN_Pos)    /* TDES OWN Bit */
#define ETH_TDES3_CTXT_Pos      (30u)
#define ETH_TDES3_CTXT          (0x1UL << ETH_TDES3_CTXT_Pos)   /* Context Type */
#define ETH_TDES3_FD_Pos        (29U)
#define ETH_TDES3_FD            (0x1UL << ETH_TDES3_FD_Pos)       /* First Descriptor */
#define ETH_TDES3_LD_Pos        (28U)
#define ETH_TDES3_LD            (0x1UL << ETH_TDES3_LD_Pos)       /* Last Descriptor */
// clang-format on

// DMA Rx descriptor control bits

/*
* There's still some control bit definitions missing- minimal implementation for now.
* Can context descriptor type & write-back descriptor type.
*/

// clang-format off
#define ETH_RDES3_OWN_Pos       (31U)
#define ETH_RDES3_OWN           (0x1UL << ETH_RDES3_OWN_Pos)    /* RDES OWN Bit */
#define ETH_RDES3_IOC_Pos       (30U)
#define ETH_RDES3_IOC           (0x1UL << ETH_RDES3_IOC_Pos)      /* Interrupt on Completion*/
#define ETH_RDES3_BUF2V_Pos     (25U)
#define ETH_RDES3_BUF2V         (0x1UL << ETH_RDES3_BUF2V_Pos)      /* Buffer 2 Address Valid */
#define ETH_RDES3_BUF1V_Pos     (24U)
#define ETH_RDES3_BUF1V         (0x1UL << ETH_RDES3_BUF1V_Pos)      /* Buffer 1 Address Valid */
// clang-format on

/** 
* Ethernet DMA Descriptor- must be word aligned
* Descriptors generally consist of 4 registers
*/
struct alignas(32) DmaDescriptor
{
    uint32_t des0{0};
    uint32_t des1{0};
    uint32_t des2{0};
    uint32_t des3{0};
};

/*
   DMA Tx Normal Desciptor Read Format
  -----------------------------------------------------------------------------------------------
  TDES0 |                         Buffer1 or Header Address  [31:0]                              |
  -----------------------------------------------------------------------------------------------
  TDES1 |                   Buffer2 Address [31:0] / Next Descriptor Address [31:0]              |
  -----------------------------------------------------------------------------------------------
  TDES2 | IOC(31) | TTSE(30) | Buff2 Length[29:16] | VTIR[15:14] | Header or Buff1 Length[13:0]  |
  -----------------------------------------------------------------------------------------------
  TDES3 | OWN(31) | CTRL[30:26] | Reserved[25:24] | CTRL[23:20] | Reserved[19:17] | Status[16:0] |
  -----------------------------------------------------------------------------------------------
*/

/** 
* @brief Config struct for tx descriptor,
* can be extended in future to support more functionality.

*/
struct TxDescriptorConfig
{
    uint32_t buff1_addr{0};
    uint32_t buff1_len{0};
    uint32_t buff2_addr{0};
    uint32_t buff2_len{0};
    bool is_start_of_packet{false};
    bool is_end_of_packet{true};
};

/** 
* @brief Wrapper around regular descriptor for TX
* @param descriptor.des0 Header or Buffer 1 Addr 
* @param descriptor.des1 Buffer 2 Addr or Buffer 1 Addr Extended
* @param descriptor.des2 Control Bits and Buffer/Header Length 
* @param descriptor.des3 More Control Bits and Frame/Payload Length
*/
class TxDmaDescriptor
{
public:
    /** 
    * @brief Configure TX descriptor
    * @param config configuration struct with buffer info and parameters
    */
    bool configure(TxDescriptorConfig& config)
    {
        // Config buffers
        descriptor.des0 = config.buff1_addr;
        uint32_t byte_length =
            (static_cast<uint32_t>(config.buff1_len) & 0x00003FFF);

        SetReg(&descriptor.des2, byte_length, ETH_TDES2_B1L_Pos, 14);

        descriptor.des1 = config.buff2_addr;
        byte_length = (static_cast<uint32_t>(config.buff2_len) & 0x00003FFF);
        SetReg(&descriptor.des2, byte_length, ETH_TDES2_B2L_Pos, 14);

        // Start/end of ethernet frame
        if (config.is_start_of_packet)
            descriptor.des3 |= ETH_TDES3_FD;

        if (config.is_end_of_packet)
        {
            descriptor.des3 |= ETH_TDES3_LD;

            // Enable interrupt on packet completion
            descriptor.des2 |= ETH_TDES2_IOC;
        }

        return true;
    }

    void set_owned_by_dma()
    {
        descriptor.des3 |= ETH_TDES3_OWN;
    }

    bool is_owned_by_cpu()
    {
        return descriptor.des3 & ETH_TDES3_OWN;
    }

    bool is_end_of_packet()
    {
        return descriptor.des3 & ETH_TDES3_LD;
    }

private:
    DmaDescriptor descriptor;
};

/**
 * @brief Class to create and manage TX descriptors, buffers with data should be created separately
 * @param tx_ring std::array of Tx descriptors
 * @param head_index Current index for descriptor list
 */

template <uint16_t Size>
class TxDescriptorManager
{
    // Make sure descriptor array stored in SRAM and word aligned
    alignas(32) static std::array<TxDmaDescriptor, Size> tx_ring
        __attribute__((section(".sram1_data")));
    uint16_t head{0};
    uint16_t tail{0};
    bool full{false};

public:
    static_assert(Size > 0, "Tx ring must have at least one descriptor");

    /**
     * @brief inserts a tx descriptor into ring
     * @param config descriptor configuration parameters
     * @returns status of operation
     */
    bool insert_desc(TxDescriptorConfig& config)
    {
        if (full)
            return false;

        // Configure buffers
        tx_ring[head].configure(config);
        head = (head + 1) % Size;

        if (head == tail)
            full = true;

        return true;
    }

    /**
     * @brief Starts transmission for ethernet packet.
     * Will transmit descriptors until finding one that 
     * marks end of a packet or until ring is empty
     * @returns new address for tail pointer, or nullptr if descriptor ring is empty
     */
    TxDmaDescriptor* send_packet()
    {
        // If descriptor ring is empty
        bool empty = (!full && head == tail);

        if (empty)
        {
            return nullptr;
        }

        while ((tx_ring[tail].is_end_of_packet() == false) && !empty)
        {
            tx_ring[tail].set_owned_by_dma();
            tail = (tail + 1) % Size;
        }

        return &tx_ring[tail];
    }
};

/*
  DMA Rx Normal Descriptor read format
  -----------------------------------------------------------------------------------------------------------
  RDES0 |                                  Buffer1 or Header Address [31:0]                                 |
  -----------------------------------------------------------------------------------------------------------
  RDES1 |                                            Reserved                                               |
  -----------------------------------------------------------------------------------------------------------
  RDES2 |                                      Payload or Buffer2 Address[31:0]                             |
  -----------------------------------------------------------------------------------------------------------
  RDES3 | OWN(31) | IOC(30) | Reserved [29:26] | BUF2V(25) | BUF1V(24) |           Reserved [23:0]          |
  -----------------------------------------------------------------------------------------------------------
*/

/** 
* @brief Wrapper around regular descriptor for RX
* @param descriptor.des0 Buffer 1 Addr
* @param descriptor.des1 Reserved
* @param descriptor.des2 Buffer 2 Addr- Should be word aligned
* @param descriptor.des3 Some control bits
*/
class RxDmaDescriptor
{
public:
    DmaDescriptor descriptor;

    void set_owned_by_dma()
    {
        descriptor.des3 |= ETH_RDES3_OWN;
    }
    bool is_owned_by_cpu()
    {
        return descriptor.des3 & ETH_RDES3_OWN;
    }
};

template <size_t Size>
class RxDescriptorManager
{
    alignas(32) static std::array<RxDmaDescriptor, Size> rx_ring
        __attribute__((section(".sram1_data")));
    uint16_t head{0};
    uint16_t tail{0};

public:
    static_assert(Size > 0, "Ring must have at least one descriptor");

    /**
     * @brief inserts a rx descriptor into rx ring
     * @returns status of operation
     */
    bool insert_desc(uint32_t addr, uint32_t addr2 = 0)
    {
        return true;
    }

    /**
     * @brief starts reception for ethernet packet data
     * @param num of descriptors used to store data
     * @returns new address for rx tail pointer
     */
    RxDmaDescriptor* receive(int num_descriptors)
    {
        return nullptr;
    }
};
}  // namespace EoT::StmH7