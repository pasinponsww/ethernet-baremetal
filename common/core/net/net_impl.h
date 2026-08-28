/**
 * @file net_impl.h
 * @brief Template implementation for EoT::Net::NetIf<TEth>
 * @note Included from the bottom of net.h so the template body stays
 *       header-visible at every instantiation point (same convention as
 *       lan8742.h including lan8742.cc). Not meant to be included directly.
 *
 *       Bodies below are stubs -- fill in the TODOs. See net.h's doc
 *       comments on each declaration for the intended behavior, and the
 *       plan doc for the full link-state transition table.
 */

#pragma once

namespace EoT::Net
{

template <typename TEth>
bool NetIf<TEth>::init(const NetConfig& config)
{
    this->config = config;

    // TODO: eth.init() -> eth.set_mac_address(config.mac_address) ->
    // eth.start(); on any failure set state = LinkState::Fault and
    // return false.

    // TODO: nif.state = this;
    // TODO: netif_add(&nif, ..., this, &NetIf::low_level_init, &ethernet_input);
    // TODO: netif_set_default(&nif) / netif_set_up(&nif) as appropriate.

    // TODO: branch on config.ip_mode:
    //   IpMode::Dhcp   -> dhcp_start(&nif)
    //   IpMode::Static -> netif_set_addr(&nif, &config.static_ip.address,
    //                       &config.static_ip.netmask, &config.static_ip.gateway)

    state = LinkState::Down;

    // TODO: run one unforced link sample here (same body poll() runs on
    // its rate-limited tick) so an already-plugged-in cable promotes
    // Down -> Up within this same call instead of waiting for poll().

    return false;  // TODO: return true once the above is implemented
}

template <typename TEth>
void NetIf<TEth>::poll()
{
    if (state == LinkState::Uninit || state == LinkState::Fault)
    {
        return;
    }

    // TODO: service_rx() -- unconditional, every call.
    // TODO: sys_check_timeouts() -- unconditional, every call.

    // TODO: rate-limit against Utils::get_ms_ticks() vs
    // last_link_poll_ms + config.link_poll_interval_ms; skip the block
    // below if the interval hasn't elapsed.

    // PhySettings sampled{};
    // bool link_ok = eth.current_link_settings(sampled);
    //
    // TODO: drive the transition table:
    //   state == Down && link_ok                          -> apply_link_up(sampled)
    //   state == Up   && !link_ok                          -> apply_link_down()
    //   state == Up   && link_ok && sampled != applied_settings (defensive)
    //                                                        -> apply_link_up(sampled)
    //   else -> no-op
}

template <typename TEth>
bool NetIf<TEth>::current_applied_settings(PhySettings& out) const
{
    if (state != LinkState::Up)
    {
        return false;
    }

    out = applied_settings;
    return true;
}

template <typename TEth>
err_t NetIf<TEth>::low_level_init(::netif* n)
{
    (void)n;
    // TODO: n->hwaddr_len = 6; memcpy(n->hwaddr, config.mac_address.data(), 6);
    // TODO: n->mtu = 1500;
    // TODO: n->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;
    // TODO: n->output = etharp_output;
    // TODO: n->linkoutput = &NetIf::low_level_output;
    // TODO: if (config.hostname) n->hostname = config.hostname;
    return ERR_OK;
}

template <typename TEth>
err_t NetIf<TEth>::low_level_output(::netif* n, pbuf* p)
{
    auto* self = static_cast<NetIf*>(n->state);
    (void)self;
    (void)p;

    // TODO: pbuf_copy_partial(p, self->tx_buf.data(), p->tot_len, 0);

    // TODO: auto status = self->eth.transmit(std::span(self->tx_buf.data(), p->tot_len));

    // TODO: map EthernetStatus -> err_t (see net.h doc comment on this method)

    return ERR_IF;
}

template <typename TEth>
void NetIf<TEth>::service_rx()
{
    // TODO: loop:
    //   std::array<uint8_t, 1518> rx_buf{};
    //   size_t len = 0;
    //   if (eth.receive(rx_buf, len) != EthernetStatus::Ok) return;
    //   pbuf* p = pbuf_alloc(PBUF_RAW, static_cast<u16_t>(len), PBUF_POOL);
    //   if (!p) return;
    //   pbuf_take(p, rx_buf.data(), static_cast<u16_t>(len));
    //   if (nif.input(p, &nif) != ERR_OK) pbuf_free(p);
}

template <typename TEth>
void NetIf<TEth>::apply_link_up(const PhySettings& s)
{
    (void)s;
    // TODO: eth.start(); (safe to call again -- see net.h doc comment)
    applied_settings = s;
    // TODO: netif_set_link_up(&nif);
    state = LinkState::Up;
}

template <typename TEth>
void NetIf<TEth>::apply_link_down()
{
    // TODO: netif_set_link_down(&nif);
    state = LinkState::Down;
}

}  // namespace EoT::Net
