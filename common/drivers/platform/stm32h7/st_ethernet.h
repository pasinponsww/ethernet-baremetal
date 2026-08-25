/**
 * @file st_ethernet.h
 * @brief STM32H7 Ethernet driver header file.
 * @author Bex Sawetrattanathumrong
 * @date 8/24/2026
 * @note This is the generic interface for the Ethernet driver layer that coupled instance of PHY, MAC, and DMA descriptor together
 */

#pragma once
#include "ethernet.h"
#include "st_eth_mac.h"
// #include "st_eth_dma.h"
#include "lan8742.h"

namespace EoT::StmH7
{

enum class EthernetStatus : uint8_t
{
    Ok = 0,
    LinkDown,
    NoBuffer,
    Timeout,
    MacError,
    PhyError,
};

struct StEthernetParams
{
    StEthMacParams mac_params;
    // StEthDmaParams dma_params; 
    // Lan8742Params<StEthMdio> phy_params;
};

struct StEthernetConfig
{
    ETH_TypeDef* base_addr;  
    StEthernetParams params;
};

class StEthernet : public Ethernet<StEthernet>
{
public:

    explicit StEthernet(const StEthernetConfig& config);
    
    /**
    * @brief Initialize the Ethernet driver and its PHY, MAC, and DMA descriptor ring
    * @return true if initialization was successful, false otherwise
    */
    bool init();

    /**
    * @brief Set the MAC address used for perfect filtering
    * @param address The MAC address to set
    * @return true if the MAC address was set successfully, false otherwise
    */
    bool set_mac_address(const MacAddress& address);

    /**
    * @brief Start the Ethernet driver
    * @return true if the driver was started successfully, false otherwise
    */
    bool start();

    /**
    * @brief Stop the Ethernet driver
    * @return true if the driver was stopped successfully, false otherwise
    */
    bool stop();
    
    /**
    * @brief Transmit a frame via the DMA descriptor ring
    * @param frame The frame bytes to transmit (header through payload)
    * @return EthernetStatus::Ok on success, otherwise the reason for failure
    */
    EthernetStatus transmit(std::span<const uint8_t> frame);

    /**
    * @brief Receive a frame via the DMA descriptor ring
    * @param frame The buffer to store the received frame
    * @param length The length of the received frame
    * @return true if a frame was received successfully, false otherwise
    */
    EthernetStatus receive(std::span<uint8_t> frame, size_t& length);

    /**
    * @brief Check if the Ethernet link is up
    * @return true if the link is up, false otherwise
    */
    bool is_link_up() const;

    /**
    * @brief Get the current link settings from the PHY
    * @param link_settings The structure to fill with the current link settings
    * @return true if the link settings were retrieved successfully, false otherwise
    */
    bool current_link_settings(PhySettings& link_settings) const;
    
private:

    StEthernetConfig params;
    ETH_TypeDef* base_addr;
    StEthMac mac;
    // StEthDma dma;
    // Lan8742<StEthMdio> phy;
};
} // namespace EoT::StmH7