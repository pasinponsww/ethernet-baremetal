/**
 * @file st_eth.h
 * @brief STM32H7 Ethernet driver
 * @author Bex Saw
 * @date 8/25/2026
 * @note Concrete composition of the MAC, PHY, DMA, and MTL layers for this
 *       MCU, satisfying the generic Ethernet<T> interface (ethernet.h).
 *       No real DMA/MTL descriptor-ring driver exists for this MCU yet --
 *       EthDma/EthMtl below are placeholder stand-ins with the minimal
 *       init()/transmit()/receive() shape StEthernet expects. Swap them
 *       for real drivers once they exist; StEthernet's public interface
 *       doesn't change.
 */

#pragma once
#include <cstddef>
#include <span>
#include "ethernet.h"
#include "lan8742.h"
#include "st_eth_dma.h"
#include "st_eth_mac.h"
#include "st_eth_mdio.h"
#include "st_eth_mtl.h"

namespace EoT::StmH7
{

class StEthernet : public Ethernet<StEthernet>
{
public:
    StEthernet(StEthMac& mac, Lan8742<StEthMdio>& phy, EthDma& dma,
               EthMtl& mtl);

    /**
    * @brief Initialize the MTL, MAC, DMA, and PHY, in that dependency order
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
    * @brief Apply the PHY's currently negotiated link settings to the MAC
    *        (best-effort -- proceeds even if the link is currently down)
    *        and enable the MAC transmitter/receiver
    * @return true if the operation was successful, false otherwise
    */
    bool start();

    /**
    * @brief Disable the MAC transmitter/receiver
    * @return true if the operation was successful, false otherwise
    */
    bool stop();

    /**
    * @brief Hand a frame off to the DMA for transmission
    * @param frame The frame bytes to transmit (header through payload)
    * @return EthernetStatus::LinkDown if start() hasn't been called yet
    *         (mac transmitter disabled), otherwise Ok/NoBuffer depending
    *         on what the DMA reports
    */
    EthernetStatus transmit(std::span<const uint8_t> frame);

    /**
    * @brief Copy the next received frame out of the DMA
    * @param buffer Destination buffer; must be large enough for the frame
    * @param[out] len Filled with the number of bytes written to buffer
    * @return EthernetStatus::LinkDown if start() hasn't been called yet
    *         (mac receiver disabled), otherwise Ok/NoBuffer depending on
    *         what the DMA reports
    */
    EthernetStatus receive(std::span<uint8_t> buffer, size_t& len);

    /**
    * @brief Check whether the PHY reports the link as up
    * @return true if the link is up, false otherwise
    */
    bool is_link_up() const;

    /**
    * @brief Read the PHY's currently negotiated (or forced) speed/duplex
    * @param[out] out Filled with the current link settings on success
    * @return true on success, false if the link is down or the read failed
    */
    bool current_link_settings(PhySettings& out) const;

private:
    StEthMac& mac;
    Lan8742<StEthMdio>& phy;
    EthDma& dma;
    EthMtl& mtl;
};

}  // namespace EoT::StmH7
