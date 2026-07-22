/**
* @file lan8742.h
* @brief LAN8742 PHY driver interface
*
* Lan8742 is a class template parameterised on the concrete MDIO backend
* (EthMdio<T>), so its definitions must be visible at every instantiation
* point. The implementation lives in lan8742.cc, which is included at the
* bottom of this header rather than compiled as a standalone translation unit.
*/

#pragma once

#include <cstdint>
#include "eth_mdio.h"

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

/**
* @brief BCR (Basic Control Register) bit positions.
* Pairs with SetReg(reg, val, bit, len).
*/
enum class BcrBit : uint32_t
{
    Reset = 15,
    Loopback = 14,
    Speed100 = 13,
    AutoNegEnable = 12,
    AutoNegRestart = 9,
    Duplex = 8
};

/**
* @brief BSR (Basic Status Register) bit positions.
*/
enum class BsrBit : uint32_t
{
    AutoNegDone = 5,
    LinkUp = 2
};

/**
* @brief Special Control/Status Register speed/duplex field (bits [4:2]).
* Mask the raw register value with kScsrModeMask, then compare against these.
*/
enum class ScsrMode : uint16_t
{
    Speed10Half = 0x0004,
    Speed100Half = 0x0008,
    Speed10Full = 0x0014,
    Speed100Full = 0x0018
};
constexpr uint16_t kScsrModeMask = 0x001CU;

// PHY ID for a LAN8742 (PHYID1:PHYID2). Low nibble is silicon revision,
// masked out when comparing.
constexpr uint32_t kLan8742Id = 0x0007C130U;
constexpr uint32_t kLan8742IdMask = 0xFFFFFFF0U;

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
    * @brief Initialize the LAN8742 PHY: soft reset, verify ID, kick auto-neg.
    * @return true if the PHY was initialized successfully, false otherwise
    */
    bool init();

    /**
    * @brief Soft-reset the PHY and wait for the reset bit to self-clear.
    * @return true if the PHY reset completed, false otherwise
    */
    bool reset();

    /**
    * @brief Read the 32-bit PHY ID (PHYID1:PHYID2).
    * @param[out] id The 32-bit PHY ID
    * @return true if the PHY ID was read successfully, false otherwise
    */
    bool read_id(uint32_t& id);

    /**
    * @brief Check whether the link is up (BSR link-status bit).
    * @return true if the link is up, false otherwise
    * @note BSR link-status latches low: a transient drop is held until the next
    *       read. Read twice if you need the instantaneous state.
    */
    bool is_link_up();

    /**
    * @brief Enable and restart auto-negotiation (non-blocking).
    * @return true if the request was written successfully, false otherwise
    */
    bool restart_auto_negotiation();

    /**
    * @brief Enable auto-negotiation (non-blocking).
    * @return true if the request was written successfully, false otherwise
    */
    bool start_auto_negotiation();

    /**
    * @brief Check whether auto-negotiation has completed (BSR bit 5).
    * @return true if auto-negotiation completed, false otherwise
    */
    bool is_auto_negotiation_done();

    /**
    * @brief Check whether a 32-bit PHY ID matches the LAN8742 (revision masked).
    * @param id The 32-bit PHY ID
    * @return true if the PHY ID is valid, false otherwise
    */
    bool is_valid_id(uint32_t id) const;

    /**
    * @brief Decode the negotiated speed/duplex from the vendor Special
    *        Control/Status register (reg 31).
    * @param[out] out Filled with the current speed and duplex on success.
    * @return PhyStatus::Ok on success, PhyStatus::LinkDown if no link,
    *         PhyStatus::MdioError on a bus error.
    */
    PhyStatus current_link_state(PhySettings& out) const;

    /**
    * @brief Enable or disable internal digital loopback (BCR bit 14).
    *
    * Loops TX back to RX inside the PHY, ahead of the line driver, so a MAC
    * can be exercised without a cable or link partner. Auto-negotiation
    * state is left untouched; call force_link_settings() first if you need
    * a specific fixed speed/duplex while looped back, since a real link
    * partner isn't present to negotiate with.
    * @param enable true to enable loopback, false to return to normal operation
    * @return true if the request was written successfully, false otherwise
    * @note While loopback is enabled, is_link_up()/current_link_state() no
    *       longer reflect a real link and should not be relied on.
    */
    bool set_loopback(bool enable);

    /**
    * @brief Force a fixed speed/duplex, bypassing auto-negotiation.
    *
    * Disables auto-negotiation (BCR bit 12) and sets the speed-select (BCR
    * bit 13) and duplex-mode (BCR bit 8) bits from the PhySettings supplied
    * at construction. Useful for loopback testing or a fixed MAC-to-MAC
    * link where there is no negotiation partner.
    * @return true if the request was written successfully, false otherwise
    */
    bool force_link_settings();

private:
    EthMdio<T>& mdio;
    uint8_t phy_addr;
    PhySettings settings;
};

}  // namespace EoT

// Template implementation. Included (not separately compiled) so the
// definitions are visible wherever Lan8742<T> is instantiated.
#include "lan8742.cc"
