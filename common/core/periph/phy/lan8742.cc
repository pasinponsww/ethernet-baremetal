#include "lan8742.h"

namespace EoT
{

// LAN8742 PHY ID
static constexpr uint32_t PHY_ID{0x0007C130}; 
// Mask for PHY ID comparison
static constexpr uint32_t PHY_ID_MASK{0xFFFFFFF0};  
static constexpr uint32_t MaxPolls{100};

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

    // Read the PHY ID
    if (!read_id(id))
    {
        return false;
    }

    // Check if the PHY ID is valid
    if (!is_valid_id(id))
    {
        return false;
    }

    // Restart auto-negotiation
    if (!restart_auto_negotiation())
    {
        return false;
    }

    return true;
}

template <typename T>
bool Lan8742<T>::reset()
{
    // Write to the Basic Control Register (BMCR) to initiate a reset
    uint16_t bmcr = 0;

    // Read the current value of the BMCR register
    if (!mdio.read(phy_addr, PhyReg::BCR, bmcr))
    {
        return false;
    }

    // Set the reset bit (bit 15) in the BMCR register
    SetReg(bmcr, 1U, 15U, 1U);

    // Write the modified value back to the BMCR register
    if (!mdio.write(phy_addr, PhyReg::BCR, bmcr))
    {
        return false;
    }

    // Poll the BMCR register until the reset bit is cleared or a timeout occurs
    for (uint32_t i = 0; i < MaxPolls; ++i)
    {
        // Read the current value of the BMCR register
        if (!mdio.read(phy_addr, PhyReg::BCR, bmcr))
        {
            return false;
        }

        // Check if the reset bit (bit 15) is cleared, indicating that the reset is complete
        if ((bmcr & (1U << 15U)) == 0U)
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

    if (!mdio.read(phy_addr, PhyReg::PHYID1, id1))
    {
        return false;
    }

    if (!mdio.read(phy_addr, PhyReg::PHYID2, id2))
    {
        return false;
    }

    // Combine the two 16-bit ID registers into a single 32-bit ID
   id = (static_cast<uint32_t>(id1) << 16) | static_cast<uint32_t>(id2);

    return true;
}

template <typename T>
bool Lan8742<T>::is_link_up()
{
    uint16_t link_status = 0;

    // read status register
    mdio.read(phy_addr, PhyReg::BSR, link_status);

    // check link status bit
    SetReg(link_status, 1U, 2U, 1U);

    if(link_status & (1U << 2U))
    {
        return true;
    }

    return false;
}

template <typename T>
bool Lan8742<T>::restart_auto_negotiation()
{
    uint16_t bmcr = 0;

    if (!mdio.read(phy_addr, PhyReg::BCR, bmcr))
    {
        return false;
    }


    SetReg(bmcr, 1U, 12U, 1U);
    SetReg(bmcr, 1U, 9U, 1U);

    if (!mdio.write(phy_addr, PhyReg::BCR, bmcr))
    {
        return false;
    }

    for (uint32_t i = 0; i < MaxPolls; ++i)
    {
        uint16_t bmsr = 0;

        if (!mdio.read(phy_addr, PhyReg::BSR, bmsr))
        {
            return false;
        }

        if ((bmsr & (1U << 5U)) != 0U)
        {
            return true;
        }
    }

    return false;
}

template <typename T>
PhyStatus Lan8742<T>::current_link_state(const PhyParams<T>& params) const
{
    // read status register
    uint16_t link_status = 0;
    mdio.read(phy_addr, PhyReg::BSR, link_status);

    // decode speed and duplex
    if ((link_status & (1U << 14U)) != 0U)
    {
        params.settings.speed = PhySpeed::Speed100M;
    }
    else
    {
        params.settings.speed = PhySpeed::Speed10M;
    }

    // return true if link is up, false otherwise
    return PhyStatus::Ok;
};

template <typename T>
bool Lan8742<T>::is_valid_id(uint32_t id) const
{
    return (id & PHY_ID_MASK) == (PHY_ID & PHY_ID_MASK);
}

}  // namespace EoT