#include "lan8742.h"
#include "reg_helpers.h"

namespace EoT
{

static constexpr uint32_t kMaxPolls{1000U};

template <typename T>
Lan8742<T>::Lan8742(PhyParams<T>& params)
    : mdio(params.mdio), phy_addr(params.phy_addr), settings(params.settings)
{
}

template <typename T>
bool Lan8742<T>::init()
{
    uint32_t id = 0;

    // Reset the PHY
    if (!reset())
    {
        return false;
    }

    // Read the PHY ID and confirm it's actually a LAN8742 before proceeding --
    // a wrong MDIO address or bad wiring can still return Ok with junk data.
    if (!read_id(id) || !is_valid_id(id))
    {
        return false;
    }

    // Kick off auto-negotiation. We do not block on completion here -- that can
    // take seconds and depends on a link partner being present.
    return restart_auto_negotiation();
}

template <typename T>
bool Lan8742<T>::reset()
{
    // Write to the Basic Control Register (BMCR) to initiate a reset
    uint16_t bmcr = 0;

    // Read the current BMCR value
    if (mdio.read(phy_addr, static_cast<uint8_t>(PhyReg::BCR), bmcr) !=
        EthMdioStatus::Ok)
    {
        return false;
    }

    // Set the soft reset bit
    SetReg(bmcr, 1U, static_cast<uint32_t>(BcrBit::Reset), 1U);

    // Write the modified BMCR value back to the PHY
    if (mdio.write(phy_addr, static_cast<uint8_t>(PhyReg::BCR), bmcr) !=
        EthMdioStatus::Ok)
    {
        return false;
    }

    // The soft-reset bit self-clears when the reset completes.
    for (uint32_t i = 0; i < kMaxPolls; ++i)
    {
        // Read the BMCR register to check if the reset has completed
        if (mdio.read(phy_addr, static_cast<uint8_t>(PhyReg::BCR), bmcr) !=
            EthMdioStatus::Ok)
        {
            return false;
        }

        // Check if the soft-reset bit is cleared
        if ((bmcr & (1U << static_cast<uint32_t>(BcrBit::Reset))) == 0U)
        {
            return true;
        }
    }

    return false;
}

template <typename T>
bool Lan8742<T>::read_id(uint32_t& id)
{
    uint16_t id1 = 0;
    uint16_t id2 = 0;

    // Read the PHYID1 and PHYID2 registers to get the 32-bit PHY ID
    if (mdio.read(phy_addr, static_cast<uint8_t>(PhyReg::PHYID1), id1) !=
        EthMdioStatus::Ok)
    {
        return false;
    }

    // Read the PHYID2 register
    if (mdio.read(phy_addr, static_cast<uint8_t>(PhyReg::PHYID2), id2) !=
        EthMdioStatus::Ok)
    {
        return false;
    }

    // Combine the two 16-bit values into a single 32-bit ID
    id = (static_cast<uint32_t>(id1) << 16) | static_cast<uint32_t>(id2);
    return true;
}

template <typename T>
bool Lan8742<T>::is_link_up()
{
    uint16_t bsr = 0;

    // Read the Basic Status Register (BSR) to check the link status
    if (mdio.read(phy_addr, static_cast<uint8_t>(PhyReg::BSR), bsr) !=
        EthMdioStatus::Ok)
    {
        return false;
    }

    // Check the link status bit in the BSR. If the bit is set, the link is up; otherwise, it is down.
    return (bsr & (1U << static_cast<uint32_t>(BsrBit::LinkUp))) != 0U;
}

template <typename T>
bool Lan8742<T>::restart_auto_negotiation()
{
    uint16_t bcr = 0;

    // Read the Basic Control Register (BCR) to modify its settings
    if (mdio.read(phy_addr, static_cast<uint8_t>(PhyReg::BCR), bcr) !=
        EthMdioStatus::Ok)
    {
        return false;
    }

    // Set the Auto-Negotiation Enable and Restart bits in the BCR to initiate auto-negotiation
    SetReg(bcr, 1U, static_cast<uint32_t>(BcrBit::AutoNegEnable), 1U);
    SetReg(bcr, 1U, static_cast<uint32_t>(BcrBit::AutoNegRestart), 1U);

    return mdio.write(phy_addr, static_cast<uint8_t>(PhyReg::BCR), bcr) ==
           EthMdioStatus::Ok;
}

template <typename T>
bool Lan8742<T>::start_auto_negotiation()
{
    uint16_t bcr = 0;

    // Read the Basic Control Register (BCR) to modify its settings
    if (mdio.read(phy_addr, static_cast<uint8_t>(PhyReg::BCR), bcr) !=
        EthMdioStatus::Ok)
    {
        return false;
    }

    // Set the Auto-Negotiation Enable bit in the BCR to enable auto-negotiation
    SetReg(bcr, 1U, static_cast<uint32_t>(BcrBit::AutoNegEnable), 1U);

    // Write the modified BCR value back to the PHY to start auto-negotiation
    return mdio.write(phy_addr, static_cast<uint8_t>(PhyReg::BCR), bcr) ==
           EthMdioStatus::Ok;
}

template <typename T>
bool Lan8742<T>::is_auto_negotiation_done()
{
    uint16_t bsr = 0;

    // Read the Basic Status Register (BSR) to check the auto-negotiation status
    if (mdio.read(phy_addr, static_cast<uint8_t>(PhyReg::BSR), bsr) !=
        EthMdioStatus::Ok)
    {
        return false;
    }

    // Check the auto-negotiation complete bit in the BSR. If the bit is set,
    // auto-negotiation has completed; otherwise, it is still in progress.
    return (bsr & (1U << static_cast<uint32_t>(BsrBit::AutoNegDone))) != 0U;
}

template <typename T>
bool Lan8742<T>::is_valid_id(uint32_t id) const
{
    return (id & kLan8742IdMask) == (kLan8742Id & kLan8742IdMask);
}

template <typename T>
PhyStatus Lan8742<T>::current_link_state(PhySettings& out) const
{
    uint16_t bsr = 0;

    // Read the Basic Status Register (BSR) to check the link status
    if (mdio.read(phy_addr, static_cast<uint8_t>(PhyReg::BSR), bsr) !=
        EthMdioStatus::Ok)
    {
        return PhyStatus::MdioError;
    }

    // Check if the link is up by examining the link status bit in the BSR.
    // If the bit is not set, return LinkDown status.
    if ((bsr & (1U << static_cast<uint32_t>(BsrBit::LinkUp))) == 0U)
    {
        return PhyStatus::LinkDown;
    }

    uint16_t scsr = 0;

    // Read the Special Control/Status Register (SCSR) to
    //  determine the current speed and duplex settings
    if (mdio.read(phy_addr, static_cast<uint8_t>(PhyReg::SPECIAL_CR_REG),
                  scsr) != EthMdioStatus::Ok)
    {
        return PhyStatus::MdioError;
    }

    // Decode the speed and duplex settings from the SCSR mode field and
    // populate the output structure accordingly.
    const auto mode = static_cast<ScsrMode>(scsr & kScsrModeMask);

    out.speed =
        (mode == ScsrMode::Speed100Half || mode == ScsrMode::Speed100Full)
            ? PhySpeed::Speed100M
            : PhySpeed::Speed10M;
    out.duplex =
        (mode == ScsrMode::Speed10Full || mode == ScsrMode::Speed100Full)
            ? PhyDuplex::Full
            : PhyDuplex::Half;

    return PhyStatus::Ok;
}

template <typename T>
bool Lan8742<T>::set_loopback(bool enable)
{
    uint16_t bcr = 0;

    // Read the current BMCR value
    if (mdio.read(phy_addr, static_cast<uint8_t>(PhyReg::BCR), bcr) !=
        EthMdioStatus::Ok)
    {
        return false;
    }

    // Set or clear the loopback bit
    SetReg(bcr, enable ? 1U : 0U, static_cast<uint32_t>(BcrBit::Loopback), 1U);

    // Write the modified BMCR value back to the PHY
    return mdio.write(phy_addr, static_cast<uint8_t>(PhyReg::BCR), bcr) ==
           EthMdioStatus::Ok;
}

template <typename T>
bool Lan8742<T>::force_link_settings()
{
    uint16_t bcr = 0;

    // Read the current BMCR value
    if (mdio.read(phy_addr, static_cast<uint8_t>(PhyReg::BCR), bcr) !=
        EthMdioStatus::Ok)
    {
        return false;
    }

    // Disable auto-negotiation so the forced speed/duplex bits take effect,
    // then apply the speed/duplex requested at construction.
    SetReg(bcr, 0U, static_cast<uint32_t>(BcrBit::AutoNegEnable), 1U);
    SetReg(bcr, settings.speed == PhySpeed::Speed100M ? 1U : 0U,
           static_cast<uint32_t>(BcrBit::Speed100), 1U);
    SetReg(bcr, settings.duplex == PhyDuplex::Full ? 1U : 0U,
           static_cast<uint32_t>(BcrBit::Duplex), 1U);

    // Write the modified BMCR value back to the PHY
    return mdio.write(phy_addr, static_cast<uint8_t>(PhyReg::BCR), bcr) ==
           EthMdioStatus::Ok;
}

}  // namespace EoT
