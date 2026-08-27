#include "st_eth.h"

namespace EoT::StmH7
{

StEthernet::StEthernet(StEthMac& mac, Lan8742<StEthMdio>& phy, EthDma& dma,
                       StEthMtl& mtl)
    : mac(mac), phy(phy), dma(dma), mtl(mtl)
{
}

bool StEthernet::init()
{
    // MTL FIFOs and MAC framing/filtering must be configured before the DMA
    // rings start moving frames through them.
    if (!mtl.init())
    {
        return false;
    }

    if (!mac.init())
    {
        return false;
    }

    if (!dma.init())
    {
        return false;
    }

    // The PHY chip is expected to be present on the board regardless of
    // whether a cable is plugged in, so init() (reset + ID check + kick
    // auto-neg) succeeding here doesn't depend on link state.
    return phy.init();
}

bool StEthernet::set_mac_address(const MacAddress& address)
{
    // If the tranmitter or receiver is enabled it cannot operate on the MAC addr reg
    if (!mac.is_transmitter_enabled() || !mac.is_receiver_enabled())
    {
        return false;
    }

    return mac.set_mac_address(address);
}

bool StEthernet::start()
{
    // If this process wasn't successful, the MAC will still be left in a disabled state
    if (!phy.start_auto_negotiation())
    {
        return false;
    }

    // Best-effort: apply whatever the PHY currently reports. If the link is
    // down this still leaves the MAC on its last-configured speed/duplex --
    // matches real bring-up, where TX/RX get armed before a cable is
    // necessarily plugged in.
    PhySettings link{};
    if (phy.current_link_state(link) == PhyStatus::Ok)
    {
        mac.apply_link_settings(link.speed, link.duplex);
    }

    return mac.enable(true);
}

bool StEthernet::stop()
{
    return mac.enable(false);
}

EthernetStatus StEthernet::transmit(std::span<const uint8_t> frame)
{

    // If the transmitter isn't enabled the mac won't accept frames and transmit the frame,
    // so we can return early with a more specific error code.
    if (!mac.is_transmitter_enabled())
    {
        return EthernetStatus::LinkDown;
    }

    return dma.transmit(frame) ? EthernetStatus::Ok : EthernetStatus::NoBuffer;
}

EthernetStatus StEthernet::receive(std::span<uint8_t> buffer, size_t& len)
{
    if (!mac.is_receiver_enabled())
    {
        len = 0U;
        return EthernetStatus::LinkDown;
    }

    return dma.receive(buffer, len) ? EthernetStatus::Ok
                                    : EthernetStatus::NoBuffer;
}

bool StEthernet::is_link_up() const
{
    return phy.is_link_up();
}

bool StEthernet::current_link_settings(PhySettings& out) const
{
    return phy.current_link_state(out) == PhyStatus::Ok;
}

}  // namespace EoT::StmH7
