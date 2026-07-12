/**
* @file lan8742.h
* @brief LAN8742 PHY driver interface
*/

#pragma once

#include <cstdint>
#include "eth_mdio.h"

namespace EoT
{

enum class PhyStatus : uint8_t
{
    Ok = 0,
    MdioError,
    Timeout,
    InvalidId,
    LinkDown
};

enum class PhySpeed : uint8_t
{
    Speed10M = 0,
    Speed100M
};

enum class PhyDuplex : uint8_t
{
    Half = 0,
    Full
};

struct PhyConfig
{
    PhyStatus status;
    PhySpeed speed;
    PhyDuplex duplex;
};

template <typename T>
struct PhyParams
{
    T& mdio;
    uint8_t phy_addr;
} 

template <typename T>
class Lan8742
{
public:

    Lan8742(const PhyParams<T>& params);

    bool init();

    bool reset();

    /**
    * @brief Read the PHY ID from the LAN8742 PHY
    * @param[out] id The 32-bit PHY ID
    * @return true if the PHY ID was read successfully, false otherwise
    */
    bool read_id(uint32_t& id);

    /**
    * @brief Check if the link is up
    * @return true if the link is up, false otherwise
    */
    bool is_link_up();

    /**
    * @brief Restart auto-negotiation
    * @return true if auto-negotiation was restarted successfully, false otherwise
    */
    bool restart_auto_negotiation();

private:
    ETH_TypeDef* eth;
    
    // read_reg()
    // - wrapper around mdio.read(phy_addr, reg, data)

    // write_reg()
    // - wrapper around mdio.write(phy_addr, reg, data)

    // - set BMCR restart auto-negotiation

    // current_link_state()
    // - read LAN8742 vendor/status register
    // - decode speed
    // - decode duplex
    // - decode link up/down

private:
    EthMdio<T>& mdio;
    uint8_t phy_addr;
};

}  // namespace EoT