#include "net.h"

namespace EoT::Net
{
NetIf<TEth>::NetIf(EoT::Ethernet<TEth>& eth) : eth(eth)
{
    nif.state = LinkState::Uninit;
}

bool NetIf<TEth>::init(const NetConfig& config)
{
}

void NetIf<TEth>::poll()
{
}

bool NetIf<TEth>::current_applied_settings(PhySettings& config) const
{
}

// Private methods below here

static err_t low_level_init(::netif* n)
{
}

static err_t low_level_output(::netif* n, pbuf* p)
{
}

void service_rx()
{
}

void apply_link_up(const PhySettings& s)
{
}

void apply_link_down()
{
}

}  // namespace EoT::Net