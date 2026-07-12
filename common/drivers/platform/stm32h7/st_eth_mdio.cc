#include "st_eth_mdio.h"

namespace EoT::StmH7
{

static constexpr uint8_t kMDIO_PHY_ADDR_COUNT{32U};
static constexpr uint8_t kMDIO_REG_ADDR_COUNT{32U};
static constexpr uint8_t kETH_MACMDIOAR_CR_BitWidth{4U};
static constexpr uint32_t kMsToUs{1000U};

static inline bool valid_phy(uint8_t phy_addr)
{
    return phy_addr < kMDIO_PHY_ADDR_COUNT;
}

static inline bool valid_reg(uint8_t reg_addr)
{
    return reg_addr < kMDIO_REG_ADDR_COUNT;
}

StEthMdio::StEthMdio(const StEthMdioConfig& config)
    : base_addr(config.base_addr),
      csr_clock_hz(config.csr_clock_hz),
      timeout_us(config.timeout_us),
      settings(config.settings)
{
}

EoT::EthMdioStatus StEthMdio::init()
{
    if (base_addr == nullptr)
    {
        return EthMdioStatus::NotInitialized;
    }

    if (csr_clock_hz == 0U)
    {
        return EthMdioStatus::InvalidClock;
    }

    // Configure the MDC clock range based on the CSR clock frequency
    auto status = check_busy_routine();
    if (status != EthMdioStatus::Ok)
    {
        return status;
    }

    // Calculate the appropriate MDC clock range based on the CSR clock frequency
    SetReg(&(base_addr->MACMDIOAR), static_cast<uint32_t>(csr_clock_hz),
           ETH_MACMDIOAR_CR_Pos, kETH_MACMDIOAR_CR_BitWidth);

    // I don't have to set the clause bit if I am using Clause 22, because the default is Clause 22.
    // But if I am using Clause 45, I need to set the clause bit in the MACMDIOAR register.
    if (settings.clause == MdioClause::Clause45)
    {
        SetReg(&(base_addr->MACMDIOAR), static_cast<uint32_t>(settings.clause),
               ETH_MACMDIOAR_C45E_Pos, 1U);
    }

    return EoT::EthMdioStatus::Ok;
}

EoT::EthMdioStatus StEthMdio::read(uint8_t phy_addr, uint8_t reg_addr,
                                   uint16_t& data)
{
    if (base_addr == nullptr)
    {
        return EthMdioStatus::NotInitialized;
    }

    if (!valid_phy(phy_addr))
    {
        return EoT::EthMdioStatus::InvalidPhyAddr;
    }

    if (!valid_reg(reg_addr))
    {
        return EoT::EthMdioStatus::InvalidRegAddr;
    }

    // Check for the Busy flag
    auto status = check_busy_routine();
    if (status != EthMdioStatus::Ok)
    {
        return status;
    }

    // Set the PHY device address
    SetReg(&(base_addr->MACMDIOAR), phy_addr, ETH_MACMDIOAR_PA_Pos, 5U);

    // Set the PHY register address
    SetReg(&(base_addr->MACMDIOAR), reg_addr, ETH_MACMDIOAR_RDA_Pos, 5U);

    // Set the read mode
    SetReg(&(base_addr->MACMDIOAR),
           ETH_MACMDIOAR_MOC_RD >> ETH_MACMDIOAR_MOC_Pos, ETH_MACMDIOAR_MOC_Pos,
           2U);

    // Set the MII Busy bit
    base_addr->MACMDIOAR |= ETH_MACMDIOAR_MB;

    // Wait for the read operation to complete
    status = check_busy_routine();
    if (status != EthMdioStatus::Ok)
    {
        return status;
    }

    // Read the data from the MDIO Data Register
    data = static_cast<uint16_t>(base_addr->MACMDIODR);

    return EoT::EthMdioStatus::Ok;
}

EoT::EthMdioStatus StEthMdio::write(uint8_t phy_addr, uint8_t reg_addr,
                                    uint16_t data)
{
    if (base_addr == nullptr)
    {
        return EthMdioStatus::NotInitialized;
    }

    if (!valid_phy(phy_addr))
    {
        return EoT::EthMdioStatus::InvalidPhyAddr;
    }

    if (!valid_reg(reg_addr))
    {
        return EoT::EthMdioStatus::InvalidRegAddr;
    }

    // Check for the Busy flag
    auto status = check_busy_routine();
    if (status != EthMdioStatus::Ok)
    {
        return status;
    }

    // Prepare the MDIO Address Register value
    base_addr->MACMDIODR = static_cast<uint32_t>(data);

    // Set the PHY device address
    SetReg(&(base_addr->MACMDIOAR), phy_addr, ETH_MACMDIOAR_PA_Pos, 5U);

    // Set the PHY register address
    SetReg(&(base_addr->MACMDIOAR), reg_addr, ETH_MACMDIOAR_RDA_Pos, 5U);

    // Set the write mode
    SetReg(&(base_addr->MACMDIOAR),
           ETH_MACMDIOAR_MOC_WR >> ETH_MACMDIOAR_MOC_Pos, ETH_MACMDIOAR_MOC_Pos,
           2U);

    // Set the MII Busy bit
    base_addr->MACMDIOAR |= ETH_MACMDIOAR_MB;

    // Wait for the write operation to complete
    // The Busy bit will be cleared by the MAC when the write operation is complete
    status = check_busy_routine();
    if (status != EthMdioStatus::Ok)
    {
        return status;
    }

    return EoT::EthMdioStatus::Ok;
}

/*****************************************************************/

bool StEthMdio::is_busy() const
{
    // Check if the MDIO interface is busy
    // this logic means if the busy bit is set, then the interface is busy, and we return true.
    return (base_addr->MACMDIOAR & ETH_MACMDIOAR_MB) != 0U;
}

uint32_t StEthMdio::get_current_us() const
{
    // Get the current time in microseconds
    return EoT::Utils::get_ms_ticks() * kMsToUs;
}

EthMdioStatus StEthMdio::check_busy_routine() const
{
    uint32_t start_time = get_current_us();

    while (is_busy())
    {
        if ((get_current_us() - start_time) > timeout_us)
        {
            return EoT::EthMdioStatus::BusyTimeout;
        }
    }
    return EoT::EthMdioStatus::Ok;
}
}  // namespace EoT::StmH7