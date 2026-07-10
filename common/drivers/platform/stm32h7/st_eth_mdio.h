/**
* @file st_eth_mdio.h
* @brief STM32H7 Ethernet MDIO driver interface
* @author Bex Saw
*/

#pragma once

#include <cstdint>
#include "eth_mdio.h"
#include "stm32h723xx.h"

namespace EoT::StmH7
{

// The clock frequency of the CSR clock in Hz (datasheet section: "ETH_CSR clock")
// The timeout value for MDIO operations in microseconds

struct StEthMdioConfig
{
    ETH_TypeDef* base_addr;
    // CSR clock 
    uint32_t csr_clock_hz{0U};
    uint32_t timeout_us{1000U};
    bool preamble_suppression_{false};
    uint8_t trailing_clocks_{0U};
    bool back_to_back_{false};
};

class StEthMdio : public EoT::EthMdio<StEthMdio>
{
public:
    explicit StEthMdio(const StEthMdioConfig& config);

    /**
    * @brief Initialize the MDIO interface
    * @param config Configuration parameters for the MDIO interface
    */
    EoT::EthMdioStatus init();

    /**
    * @brief Read a register from a PHY device
    * @param phy_addr PHY address (0-31)
    * @param reg_addr Register address (0-31)
    * @param data Reference to store the read data
    * @return Status of the read operation
    */
    EoT::EthMdioStatus read(uint8_t phy_addr, uint8_t reg_addr, uint16_t& data);
    /**
    * @brief Write a register to a PHY device
    * @param phy_addr PHY address (0-31)
    * @param reg_addr Register address (0-31)
    * @param data Data to write
    * @return Status of the write operation
    */
    EoT::EthMdioStatus write(uint8_t phy_addr, uint8_t reg_addr, uint16_t data);

private:
    ETH_TypeDef* base_addr;
    uint32_t csr_clock_hz{0U};
    uint32_t timeout_us{1000U};
};

}  // namespace EoT::StmH7