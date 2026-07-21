/**
* @brief MAC Layer for STM32H7
* @author Bex Saw
* @date 7/20/2026
* @note This is the MAC Layer for ETHERNET on STM32H7. It provides an interface for 
*       initializing and configuring the MAC, as well as sending and receiving Ethernet frames.
*/

#pragma once
#include <cstdint>
#include <array>
#include "lan8742.h"
#include "eth_mac.h"

namespace EoT
{
namespace StmH7
{

// Define enums for MAC configuration settings

// Definition of all MAC specific stuff here 

struct StEthMacSettings
{
    PhySpeed speed;
    PhyDuplex duplex;
    bool loopback;
    std::array<uint8_t, 6> mac_address;
};

struct StEthMacParams
{
    StEthMacSettings settings;
    ETH_TypeDef* base_addr; 
};

class StEthMac : public Mac<StEthMac>
{
public:

    explicit StEthMac(const StEthMacParams& params);

    /**
    * @brief Initialize the MAC layer
    * @return true if initialization was successful, false otherwise
    */
    bool init();

    /**
    * @brief Program the station MAC address used for perfect-filter matching
    * @param addr 6-byte MAC address
    * @return true if the MAC address was set successfully, false otherwise
    */
    bool set_mac_address(const std::array<uint8_t, 6>& addr);

    /**
    * @brief Apply link settings
    * @param speed Link speed
    * @param duplex Link duplex mode
    * @return true if settings were applied successfully, false otherwise
    */
    bool apply_link_settings(PhySpeed speed, PhyDuplex duplex);

    /**
    * @brief Enable or disable the MAC
    * @param on true to enable, false to disable
    * @return true if the operation was successful, false otherwise
    */
    bool enable(bool on);

    /**
    * @brief Set the loopback mode
    * @param on true to enable loopback, false to disable
    * @return true if the operation was successful, false otherwise
    */
    bool set_loopback(bool on);

private:
    ETH_TypeDef* base_addr;  
    StEthMacSettings settings;
};
}
}