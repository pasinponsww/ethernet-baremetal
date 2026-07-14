#include "lan8742.h"

namespace EoT
{
Lan8742::Lan8742(const PhyParams& params)
    : mdio(params.mdio), phy_addr(params.phy_addr), settings(params.settings)
{
}

bool Lan8742::init()
{
    // read PHY ID registers
    mdio.read(); // Find the params of where is the LAN chip is acutally connected.

    // check if the ID matches LAN8742
    if(/* check ID */) 
    {
        return false; 
    }

    // configure auto-negotiation so that the PHY can 
    // negotiate speed and duplex with the link partner
    restart_auto_negotiation();

    // start auto-negotiation b/c we need to ensure the link is up before proceeding
    start_auto_negotiation();

    // wait for link up
    is_link_up();

    return true; 
}

bool Lan8742::reset()
{
    // - write to BMCR register to reset PHY
    // - wait for reset to complete
    return true; 
}

bool Lan8742::read_id(uint32_t& id)
{
    // - read PHY ID registers
    // - combine the two 16-bit registers into a 32-bit ID
    return true; 
}

bool Lan8742::is_link_up()
{
    // - read status register
    // - check link status bit
    return true; 
}

bool Lan8742::restart_auto_negotiation()
{
    // - write to BMCR register to restart auto-negotiation
    return true; 
}

bool Lan8742::current_link_state() const
{
    // - read status register
    // - decode speed and duplex
    // - return true if link is up, false otherwise
    return true; 
};

} // namespace EoT