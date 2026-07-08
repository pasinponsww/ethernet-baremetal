#include "st_mdio.h"

namespace EoT::StmH7
{

static constexpr uint32_t kMDIOS_DATA_Msk{0xFFFFU};

StMdio::StMdio(const StMdioParams& params)
    : base_addr(params.base_addr),
      port_addr(params.phy_addr),
      timeout_cycles(params.timeout_cycles)
{
}

EoT::MdioStatus StMdio::init()
{
    // Check if base address is valid
    if (base_addr == nullptr)
    {
        return EoT::MdioStatus::HARDWARE_ERROR;
    }

    // Check if the PHY address is valid
    if (!valid_phy(port_addr))
    {
        return EoT::MdioStatus::INVALID_PHY;
    }

    // Disable the MDIO interface before configuration
    base_addr->CR &= ~MDIOS_CR_EN;

    // Set the port address in the control register
    SetReg(&base_addr->CR, port_addr, MDIOS_CR_PORT_ADDRESS_Pos, 5);

    // Clear any pending interrupts and errors
    base_addr->CR &= ~(MDIOS_CR_WRIE | MDIOS_CR_RDIE | MDIOS_CR_EIE);

    // Disable the DPC bit to allow direct access to the PHY registers
    base_addr->CR &= ~MDIOS_CR_DPC;

    // Clear any pending flags in the status register
    base_addr->CLRFR = MDIOS_SR_CPERF | MDIOS_SR_CSERF | MDIOS_SR_CTERF;

    // Enable the MDIO interface
    base_addr->CR |= MDIOS_CR_EN;

    return EoT::MdioStatus::OK;
}

EoT::MdioStatus StMdio::check(uint8_t phy_addr, uint8_t reg_addr) const
{
    // Validate PHY and register addresses
    if (!valid_phy(phy_addr))
    {
        return EoT::MdioStatus::INVALID_PHY;
    }

    // Validate register address
    if (!valid_reg(reg_addr))
    {
        return EoT::MdioStatus::INVALID_REG;
    }

    // Check if the PHY address matches the configured port address
    if (phy_addr != port_addr)
    {
        return EoT::MdioStatus::INVALID_PHY;
    }

    // Check if the MDIO interface is enabled
    if ((base_addr->CR & MDIOS_CR_EN) == 0U)
    {
        return EoT::MdioStatus::BUSY;
    }

    return EoT::MdioStatus::OK;
}

EoT::MdioStatus StMdio::hardware_status() const
{
    // Check for hardware errors in the status register
    if (base_addr->SR & (MDIOS_SR_PERF | MDIOS_SR_SERF | MDIOS_SR_TERF))
    {
        // Clear the error flags in the status register
        base_addr->CLRFR = MDIOS_SR_CPERF | MDIOS_SR_CSERF | MDIOS_SR_CTERF;

        return EoT::MdioStatus::HARDWARE_ERROR;
    }

    return EoT::MdioStatus::OK;
}

EoT::MdioStatus StMdio::read(uint8_t phy_addr, uint8_t reg_addr, uint16_t& data)
{
    // Check the validity of the PHY and register addresses
    auto status = check(phy_addr, reg_addr);

    // If the check fails, return the corresponding error status
    if (status != EoT::MdioStatus::OK)
    {
        return status;
    }

    status = hardware_status();

    // If there is a hardware error, return the corresponding error status
    if (status != EoT::MdioStatus::OK)
    {
        return status;
    }

    // Read the data from the specified register and mask it to 16 bits
    data = static_cast<uint16_t>(*(&base_addr->DINR0 + reg_addr) &
                                 kMDIOS_DATA_Msk);

    return EoT::MdioStatus::OK;
}

EoT::MdioStatus StMdio::write(uint8_t phy_addr, uint8_t reg_addr, uint16_t data)
{
    // Check the validity of the PHY and register addresses
    auto status = check(phy_addr, reg_addr);

    // If the check fails, return the corresponding error status
    if (status != EoT::MdioStatus::OK)
    {
        return status;
    }

    status = hardware_status();

    // If there is a hardware error, return the corresponding error status
    if (status != EoT::MdioStatus::OK)
    {
        return status;
    }

    // Write the data to the specified register, masking it to 16 bits
    // Its not pointing to the specific register like generic DOUTR (ST provide us with the each DOUTR0...DOUTR31)
    // so we need to use pointer arithmetic to access the correct register
    *(&base_addr->DOUTR0 + reg_addr) =
        static_cast<uint32_t>(data) & kMDIOS_DATA_Msk;

    return EoT::MdioStatus::OK;
}

}  // namespace EoT::StmH7