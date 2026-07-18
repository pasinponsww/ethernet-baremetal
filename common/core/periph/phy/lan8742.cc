#include "lan8742.h"

namespace EoT
{

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
    // - write to BMCR register to reset PHY
    // - wait for reset to complete
    return true;
}

template <typename T>
bool Lan8742<T>::read_id(uint32_t& id)
{
    uint16_t id1 = 0;
    uint16_t id2 = 0;

    if (!mdio.read(phy_addr, PHY_ID1_REG, id1))
    {
        return false;
    }

    if (!mdio.read(phy_addr, PHY_ID2_REG, id2))
    {
        return false;
    }

    // Combine the two 16-bit ID registers into a single 32-bit ID
    id = (static_cast<uint32_t>(id1) << 16) | id2;

    return true;
}

template <typename T>
bool Lan8742<T>::is_link_up()
{
    // - read status register
    // - check link status bit
    return true;
}

template <typename T>
bool Lan8742<T>::restart_auto_negotiation()
{
    // - write to BMCR register to restart auto-negotiation
    return true;
}

template <typename T>
PhyStatus Lan8742<T>::current_link_state(const PhyParams<T>& params) const
{
    // - read status register
    // - decode speed and duplex
    // - return true if link is up, false otherwise
    return PhyStatus::Ok;
};

}  // namespace EoT