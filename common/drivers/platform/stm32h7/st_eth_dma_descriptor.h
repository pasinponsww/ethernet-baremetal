#include <cstddef>
#include <cstdint>

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
 * @brief Class to create and manage TX descriptors
 * @param tx_ring std::array of Tx descriptors
 * @param head_index Current index for descriptor list
 */
template <size_t Size>
class TxDescriptorManager
{
    alignas(32) std::array<Size, TxDmaDescriptor> tx_ring
        __attribute__((section(".sram1_data")));
    uint32_t head_index{0};

public:
    TxDmaDescriptor& current(){return tx_ring[head_index]};
    void advance_head()
    {
        head_index = (head_index + 1) % Size;
    }

    bool can_transmit()
    {
        return current().is_owned_by_cpu();
    }

    uint32_t prepare_next(uint8_t* data, uint32_t len)
    {
    }

    static_assert(N > 0, "Ring must have at least one descriptor");
};

/** 
* @brief Wrapper around regular descriptor for RX
* @param descriptor.des0 Buffer 1 Addr
* @param descriptor.des1 Reserved
* @param descriptor.des2 Buffer 2 Addr- Should be word aligned
* @param descriptor.des3 Some control bits
*/
class RxDmaDescriptor
{
    DmaDescriptor descriptor;
};

}  // namespace EoT::StmH7