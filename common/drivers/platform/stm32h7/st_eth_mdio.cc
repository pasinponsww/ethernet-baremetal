#include "st_eth_mdio.h"

namespace EoT::StmH7
{

StEthMdio::StEthMdio(const StEthMdioParams& params)
    : base_addr(params.base_addr),
      port_addr(params.phy_addr),
      timeout_cycles(params.timeout_cycles)
{
}

EoT::MdioStatus StEthMdio::init()
{
    // TODO: Initialized sequence from the datasheet MDIOS_ETH SPECIFC ONLY
    return EoT::MdioStatus::OK;
}

EoT::MdioStatus StEthMdio::hardware_status() const
{
   // TODO: Check the hardware status of the ETH MDIOS peripheral and return appropriate status
   // For now, we will assume the hardware is always OK

    return EoT::MdioStatus::OK;
}

EoT::MdioStatus StEthMdio::check(uint8_t phy_addr, uint8_t reg_addr) const
{
    if (base_addr == nullptr)
    {
        return EoT::MdioStatus::HARDWARE_ERROR;
    }

    if (!valid_phy(phy_addr))
    {
        return EoT::MdioStatus::INVALID_PHY;
    }

    if (!valid_reg(reg_addr))
    {
        return EoT::MdioStatus::INVALID_REG;
    }

    return EoT::MdioStatus::OK;
}

EoT::MdioStatus StEthMdio::read(uint8_t phy_addr, uint8_t reg_addr, uint16_t& data)
{
    // read sequences for the ETH_MDIOS SPECIFIC ONLY
    
    return EoT::MdioStatus::OK;
}

EoT::MdioStatus StEthMdio::write(uint8_t phy_addr, uint8_t reg_addr, uint16_t data)
{
    // write sequences for the ETH_MDIOS SPECIFIC ONLY

    return EoT::MdioStatus::OK;
}

} // namespace EoT::StmH7