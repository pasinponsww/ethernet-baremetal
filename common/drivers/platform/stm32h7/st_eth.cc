#include "st_eth.h"

namespace EoT::StmH7
{

StEthernet::StEthernet(StEthMac& mac, Lan8742<StEthMdio>& phy, EthDma& dma,
                        EthMtl& mtl)
    : mac_(mac), phy_(phy), dma_(dma), mtl_(mtl)
{
}

bool StEthernet::init()
{
    // MTL FIFOs and MAC framing/filtering must be configured before the DMA
    // rings start moving frames through them.
    if (!mtl_.init())
    {
        return false;
    }

    if (!mac_.init())
    {
        return false;
    }

    if (!dma_.init())
    {
        return false;
    }

    // The PHY chip is expected to be present on the board regardless of
    // whether a cable is plugged in, so init() (reset + ID check + kick
    // auto-neg) succeeding here doesn't depend on link state.
    return phy_.init();
}

bool StEthernet::set_mac_address(const MacAddress& address)
{
    return mac_.set_mac_address(address);
}

bool StEthernet::start()
{
    // Best-effort: apply whatever the PHY currently reports. If the link is
    // down this still leaves the MAC on its last-configured speed/duplex --
    // matches real bring-up, where TX/RX get armed before a cable is
    // necessarily plugged in.
    PhySettings link{};
    if (phy_.current_link_state(link) == PhyStatus::Ok)
    {
        mac_.apply_link_settings(link.speed, link.duplex);
    }

    return mac_.enable(true);
}

bool StEthernet::stop()
{
    return mac_.enable(false);
}

EthernetStatus StEthernet::transmit(std::span<const uint8_t> frame)
{
    // Gated on the MAC's own TX-enable (i.e. whether start() has been
    // called), not on PHY link state -- is_link_up() is the separate,
    // explicit signal for "is a partner actually there".
    if (!mac_.is_transmitter_enabled())
    {
        return EthernetStatus::LinkDown;
    }

    return dma_.transmit(frame) ? EthernetStatus::Ok
                                 : EthernetStatus::NoBuffer;
}

EthernetStatus StEthernet::receive(std::span<uint8_t> buffer, size_t& len)
{
    if (!mac_.is_receiver_enabled())
    {
        len = 0U;
        return EthernetStatus::LinkDown;
    }

    return dma_.receive(buffer, len) ? EthernetStatus::Ok
                                      : EthernetStatus::NoBuffer;
}

bool StEthernet::is_link_up() const
{
    return phy_.is_link_up();
}

bool StEthernet::current_link_settings(PhySettings& out) const
{
    return phy_.current_link_state(out) == PhyStatus::Ok;
}

}  // namespace EoT::StmH7
