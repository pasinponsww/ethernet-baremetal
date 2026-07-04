/**
* @file st_mdio.h
* @brief STM32H7 MDIO driver interface
* @author Bex Saw
*/

#include "mdio.h"
#include "stm32h723xx.h"
#include "reg_helpers.h"

namespace EoT::StmH7
{

// Define all enums 

// Struct for configuring the MDIO peripheral

class StMdio : public Mdio<StMdio>
{
public: 

    explicit StMdio(/*definition*/);

    /**
     * @brief Read a register from the MDIO interface
     * @param phy_addr The PHY address
     * @param reg_addr The register address
     * @return true if the read was successful, false otherwise
     */
    bool read(uint8_t phy_addr, uint8_t reg_addr);

    /**
     * @brief Write a value to a register in the MDIO interface
     * @param phy_addr The PHY address
     * @param reg_addr The register address
     * @param data The data to write
     * @return true if the write was successful, false otherwise
     */
    bool write(uint8_t phy_addr, uint8_t reg_addr, uint16_t data);

private:

    MDIOS_TypeDef* base_addr;

    // Helpers possibly

};
}
