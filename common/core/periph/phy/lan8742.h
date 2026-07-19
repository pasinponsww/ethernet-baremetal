/**
* @file lan8742.h
* @brief LAN8742 PHY driver interface
*/

#pragma once

#include <cstdint>
#include "eth_mdio.h"
#include "reg_helpers.h"

namespace EoT
{

/**
* @brief LAN8742 PHY register addresses
* BCR = Basic Control Register
* BSR = Basic Status Register
* PHYID1 = PHY Identifier 1
* PHYID2 = PHY Identifier 2
* ANAR = Auto-Negotiation Advertisement Register
* ANLPAR = Auto-Negotiation Link Partner Ability Register
* ANER = Auto-Negotiation Expansion Register
* ANNPTXR = Auto-Negotiation Next Page Transmit Register
* ANNPRXR = Auto-Negotiation Next Page Receive Register
* MMDACR = MMD Access Control Register
* MMDAADR = MMD Access Address Data Register
* SPECIAL_CR_REG = Special Control/Status Register
*/
enum class PhyReg : uint8_t
{
    BCR = 0,
    BSR = 1,
    PHYID1 = 2,
    PHYID2 = 3,
    ANAR = 4,
    ANLPAR = 5,
    ANER = 6,
    ANNPTXR = 7,
    ANNPRXR = 8,
    MMDACR = 13,
    MMDAADR = 14,
    SPECIAL_CR_REG = 31
};

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

struct PhySettings
{
    PhySpeed speed;
    PhyDuplex duplex;
};

template <typename T>
struct PhyParams
{
    EthMdio<T>& mdio;
    uint8_t phy_addr;
    PhySettings settings;
};

template <typename T>
class Lan8742
{
public:
    explicit Lan8742(PhyParams<T>& params);

    /**
    * @brief Initialize the LAN8742 PHY
    * @return true if the PHY was initialized successfully, false otherwise
    */
    bool init();

    /**
    * @brief Reset the LAN8742 PHY
    * @return true if the PHY was reset successfully, false otherwise
    */
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

    /**
    * @brief Start auto-negotiation
    * @return true if auto-negotiation was started successfully, false otherwise
    */
    bool start_auto_negotiation();

    /**
    * @brief Check if the PHY ID is valid
    * @param id The 32-bit PHY ID
    * @return true if the PHY ID is valid, false otherwise
    */
    bool is_valid_id(uint32_t id);

private:
    /**
    * @brief Get the current link state of the PHY
    * @param params The PHY parameters
    * @return The current link state of the PHY
    */
    PhyStatus current_link_state(const PhyParams<T>& params) const;

    EthMdio<T>& mdio;
    uint8_t phy_addr;
    PhySettings settings;
};

}  // namespace EoT