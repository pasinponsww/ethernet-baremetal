/**
* @file st_mdio.h
* @brief STM32H7 MDIOS driver interface
* @author Bex Saw
*/

#pragma once

#include "mdio.h"
#include "reg_helpers.h"
#include "stm32h723xx.h"

#include <cstdint>

namespace EoT::StmH7
{

static constexpr uint8_t MDIO_MAX_PHY_ADDR{32U};
static constexpr uint8_t MDIO_MAX_REG_ADDR{32U};

struct StMdioParams
{
    MDIOS_TypeDef* base_addr;
    uint8_t phy_addr;
    uint32_t timeout_cycles{10000U};
};

class StMdio : public EoT::Mdio<StMdio>
{
public:
    explicit StMdio(const StMdioParams& params);

    EoT::MdioStatus init();

    /**
    * @brief Read a register from the MDIO interface
    * @param phy_addr The PHY address
    * @param reg_addr The register address
    * @param data Reference to store the read data
    * @return MdioStatus indicating the result of the read operation
    */
    EoT::MdioStatus read(uint8_t phy_addr, uint8_t reg_addr, uint16_t& data);

    /**
    * @brief Write a register to the MDIO interface
    * @param phy_addr The PHY address
    * @param reg_addr The register address
    * @param data The data to write
    * @return MdioStatus indicating the result of the write operation
    */
    EoT::MdioStatus write(uint8_t phy_addr, uint8_t reg_addr, uint16_t data);

private:
    MDIOS_TypeDef* base_addr;
    uint8_t port_addr;
    uint32_t timeout_cycles;

    bool valid_phy(uint8_t phy_addr) const
    {
        return phy_addr < MDIO_MAX_PHY_ADDR;
    }

    bool valid_reg(uint8_t reg_addr) const
    {
        return reg_addr < MDIO_MAX_REG_ADDR;
    }

    /**
    * @brief Waits for the MDIO interface to be ready for the next operation
    * @return true if ready, false if timeout occurred
    */
    EoT::MdioStatus check(uint8_t phy_addr, uint8_t reg_addr) const;

    /**
    * @brief Gets the hardware status of the MDIO interface
    * @return MdioStatus indicating the hardware status
    */
    EoT::MdioStatus hardware_status() const;
};

}  // namespace EoT::StmH7