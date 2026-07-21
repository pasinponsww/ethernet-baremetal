#include "st_eth_mac.h"

namespace EoT::StmH7
{
StEthMac::StEthMac(const StEthMacParams& params)
    : base_addr(params.base_addr), settings(params.settings)
{
}

bool StEthMac::init()
{
}

bool StEthMac::set_mac_address(const std::array<uint8_t, 6>& addr)
{
}

bool StEthMac::apply_link_settings(PhySpeed speed, PhyDuplex duplex)
{
}

bool StEthMac::enable(bool on)
{
}

bool StEthMac::set_loopback(bool on)
{
}

}