/**
* @brief SMA Ethernet (Station management agent for PHY) interface
* @file sma.h
* @author Bex Saw
* @date 7/9/2026
*/

#pragma once
// #include "eth_mdio.h"
// #include "st_eth_mdio.h"
// #include "phy.h"

// NOTE: Essentially, We going to have the interface where the SMA talking with the
// PHY + RMII that is connected to the PHY (but we have a seperate driver for that)
// NOTE: The SMA is going to be a wrapper for the MDIO driver, and the RMII driver is going to be a wrapper for the PHY driver
namespace EoT
{
class SMA
{
public:
private:
};
}  // namespace EoT