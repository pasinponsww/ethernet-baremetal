/**
 * @file net_impl.h
 * @brief Template implementation for EoT::Net::NetIf<TEth>
 * @note Included from the bottom of net.h so the template body stays
 *       header-visible at every instantiation point (same convention as
 *       lan8742.h including lan8742.cc). Not meant to be included directly.
 *
 *       See net.h for the contract each method is implementing; this file
 *       carries only the notes that are about the code rather than the
 *       design.
 *
 *       Everything here runs in one context -- the superloop, via poll().
 *       There are no interrupts feeding lwIP and no RTOS (NO_SYS=1), so
 *       none of this needs locking, and equally none of it may be called
 *       from an ISR.
 *
 * @author Bex Saw
 */

#pragma once

#include <cstring>  // memcpy, for the hwaddr copy in low_level_init

#include "delay.h"           // EoT::Utils::get_ms_ticks()
#include "lwip/dhcp.h"       // dhcp_start
#include "lwip/init.h"       // lwip_init
#include "lwip/timeouts.h"   // sys_check_timeouts
#include "netif/etharp.h"    // etharp_output
#include "netif/ethernet.h"  // ethernet_input

namespace EoT::Net
{

namespace
{

/**
 * @brief Upper bound on frames drained from the driver per poll() call
 * @note service_rx() would otherwise keep looping for as long as frames
 *       keep arriving, which on a busy segment means the rest of the
 *       superloop never runs again. 16 frames is roughly 24 KiB of traffic
 *       per tick -- far more than a 100 Mbit link delivers in a 1 ms tick,
 *       so this only ever bites during a burst or a broadcast storm, which
 *       is exactly when the application most needs to keep running.
 */
constexpr uint32_t kMaxRxFramesPerPoll{16U};

/**
 * @brief Translate a driver-level result into the err_t lwIP expects back
 *        from netif->linkoutput
 * @note See net.h for why each case maps the way it does. ERR_MEM is the
 *       only value lwIP treats as "try again later" -- TCP will requeue the
 *       segment and retransmit. Everything else is reported to the upper
 *       layer as a hard interface error, which is the honest answer for a
 *       dead PHY or an unimplemented DMA.
 */
constexpr err_t to_lwip_err(EthernetStatus status)
{
    switch (status)
    {
        case EthernetStatus::Ok:
            return ERR_OK;

        case EthernetStatus::NoBuffer:
            return ERR_MEM;

        case EthernetStatus::Timeout:
            return ERR_TIMEOUT;

        case EthernetStatus::InvalidArgument:
            return ERR_ARG;

        case EthernetStatus::LinkDown:
        case EthernetStatus::MacError:
        case EthernetStatus::PhyError:
        case EthernetStatus::NotSupported:
        case EthernetStatus::NoFrame:
        default:
            return ERR_IF;
    }
}

}  // namespace

/**
 * @brief One-time lwip_init() guard, shared by every NetIf<TEth> instantiation
 * @note Deliberately NOT a static local inside NetIf<TEth>::init() itself:
 *       init() is a template method, so each distinct TEth would get its
 *       own independent static, and two ports behind two different driver
 *       types would each believe they were first and call lwip_init()
 *       twice, resetting lwIP's memory pools and timer list out from under
 *       whichever port initialized first. Also deliberately NOT inside the
 *       anonymous namespace above: that gives internal linkage, so each
 *       translation unit would again get its own copy. An ordinary inline
 *       function has external, vague linkage -- its local static is merged
 *       by the linker into one instance shared by every TEth and every TU.
 */
inline bool& lwip_started_flag()
{
    static bool started{false};
    return started;
}

template <typename TEth>
bool NetIf<TEth>::init(const NetConfig& config)
{
    this->config = config;

    // Hardware first, and strictly in this order: the MAC address has to be
    // in the filter before the receiver is enabled, or the first frames
    // after start() are filtered against a zero address.
    if (!eth.init() || !eth.set_mac_address(config.mac_address) || !eth.start())
    {
        state = LinkState::Fault;
        return false;
    }

    // lwip_init() sets up the memory pools and timer list and must run
    // exactly once per boot, before any other lwIP call.
    bool& lwip_started{lwip_started_flag()};
    if (!lwip_started)
    {
        lwip_init();
        lwip_started = true;
    }

    // Static addressing hands the real values to netif_add; DHCP starts
    // from all-zero and dhcp_start() fills them in when a lease arrives.
    ip4_addr_t address{};
    ip4_addr_t netmask{};
    ip4_addr_t gateway{};

    if (config.ip_mode == IpMode::Static)
    {
        address = config.static_ip.address;
        netmask = config.static_ip.netmask;
        gateway = config.static_ip.gateway;
    }

    // `this` becomes netif->state, which is how the two static callbacks
    // below find their way back to this object. netif_add() assigns it
    // before invoking low_level_init, so that callback can already use it.
    //
    // ethernet_input (not ip4_input) is the input function: it is what
    // demuxes on EtherType, so ARP replies reach etharp instead of being
    // handed to the IP layer as malformed packets.
    if (netif_add(&nif, &address, &netmask, &gateway, this,
                  &NetIf::low_level_init, &ethernet_input) == nullptr)
    {
        state = LinkState::Fault;
        return false;
    }

    // Default route: the only interface on the board, so everything that
    // is not link-local goes out of it.
    netif_set_default(&nif);

    // Administratively up. Distinct from link up -- this says the software
    // interface is enabled, while the link flag (driven from sample_link()
    // below) says whether a cable is actually carrying anything.
    netif_set_up(&nif);

    if (config.ip_mode == IpMode::Dhcp)
    {
        // LWIP_DHCP_CHECK_LINK_UP is on, so this arms the client but sends
        // nothing until netif_set_link_up() -- no discovery packets are
        // wasted on an unplugged cable, and replugging restarts discovery
        // on its own.
        if (dhcp_start(&nif) != ERR_OK)
        {
            state = LinkState::Fault;
            return false;
        }
    }

    state = LinkState::Down;
    last_link_poll_ms = Utils::get_ms_ticks();

    // One unforced pass of the same transition table poll() runs, so a
    // cable that is already plugged in comes up inside this call rather
    // than one link_poll_interval_ms later.
    sample_link();

    return true;
}

template <typename TEth>
void NetIf<TEth>::poll()
{
    if (state == LinkState::Uninit || state == LinkState::Fault)
    {
        return;
    }

    // Every tick, unconditionally. These two are what actually make lwIP
    // run: without service_rx() nothing is ever received, and without
    // sys_check_timeouts() no ARP entry ever ages out, no DHCP lease is
    // ever renewed and no TCP segment is ever retransmitted.
    service_rx();
    sys_check_timeouts();

    // The link sample costs an MDIO transaction (tens of microseconds), so
    // it is rate-limited rather than run every tick. Unsigned subtraction
    // is deliberate: it stays correct across the ~49.7-day wrap of the
    // millisecond tick, which comparing `now > last + interval` would not.
    const uint32_t now = Utils::get_ms_ticks();
    if ((now - last_link_poll_ms) < config.link_poll_interval_ms)
    {
        return;
    }

    last_link_poll_ms = now;
    sample_link();
}

template <typename TEth>
void NetIf<TEth>::sample_link()
{
    PhySettings sampled{};
    const bool link_ok = eth.current_link_settings(sampled);

    if (!link_ok)
    {
        if (state == LinkState::Up)
        {
            apply_link_down();
        }

        return;
    }

    if (state != LinkState::Up)
    {
        apply_link_up(sampled);
        return;
    }

    // Already up. Renegotiation while the link stays nominally up is rare
    // but real -- a switch port reconfigured from 100M full to 10M half,
    // for instance -- and leaves the MAC clocking at the old rate, which
    // corrupts every frame. Cheap to check, so check.
    if (sampled.speed != applied_settings.speed ||
        sampled.duplex != applied_settings.duplex)
    {
        apply_link_up(sampled);
    }
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
    auto* self = static_cast<NetIf*>(n->state);

    // Two-character interface name lwIP uses in its own diagnostics.
    n->name[0] = 'e';
    n->name[1] = '0';

    n->hwaddr_len = ETH_HWADDR_LEN;
    std::memcpy(n->hwaddr, self->config.mac_address.data(), ETH_HWADDR_LEN);

    // Payload only -- the 14-byte Ethernet header is not counted here.
    n->mtu = 1500U;

    // BROADCAST so DHCP and ARP requests can be sent to ff:ff:ff:ff:ff:ff,
    // ETHARP so lwIP resolves IPv4 destinations to MAC addresses rather
    // than expecting us to. NETIF_FLAG_LINK_UP is intentionally absent --
    // see net.h.
    n->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;

    // output is the IP-layer entry point: it takes an IP packet and runs
    // ARP resolution, eventually calling linkoutput with a finished frame.
    // linkoutput is ours and is handed a complete Ethernet frame.
    n->output = etharp_output;
    n->linkoutput = &NetIf::low_level_output;

#if LWIP_NETIF_HOSTNAME
    if (self->config.hostname != nullptr)
    {
        n->hostname = self->config.hostname;
    }
#endif

    return ERR_OK;
}

template <typename TEth>
err_t NetIf<TEth>::low_level_output(::netif* n, pbuf* p)
{
    auto* self = static_cast<NetIf*>(n->state);

    if (p->tot_len == 0U || p->tot_len > self->tx_buf.size())
    {
        return ERR_ARG;
    }

    // lwIP may hand over a chain (header in one pbuf, payload in another);
    // Ethernet<T>::transmit takes a single contiguous span, so flatten it.
    // pbuf_copy_partial walks the chain and returns what it actually
    // copied, which is the only way to notice a malformed chain.
    const u16_t copied =
        pbuf_copy_partial(p, self->tx_buf.data(), p->tot_len, 0U);

    if (copied != p->tot_len)
    {
        return ERR_BUF;
    }

    const EthernetStatus status = self->eth.transmit(std::span<const uint8_t>(
        self->tx_buf.data(), static_cast<size_t>(copied)));

    return to_lwip_err(status);
}

template <typename TEth>
void NetIf<TEth>::service_rx()
{
    for (uint32_t drained = 0U; drained < kMaxRxFramesPerPoll; ++drained)
    {
        size_t length{0U};

        if (eth.receive(rx_buf, length) != EthernetStatus::Ok)
        {
            // NoFrame -- the normal idle exit -- and any genuine driver
            // error both end the drain. Retrying inside this loop would
            // only spin.
            return;
        }

        if (length == 0U || length > rx_buf.size())
        {
            return;
        }

        pbuf* p = pbuf_alloc(PBUF_RAW, static_cast<u16_t>(length), PBUF_POOL);
        if (p == nullptr)
        {
            // Pool exhausted. Drop this frame and stop: the pool only
            // refills when the upper layers free what they already hold,
            // which cannot happen from inside this loop. Ethernet is
            // lossy by design and TCP will retransmit.
            return;
        }

        if (pbuf_take(p, rx_buf.data(), static_cast<u16_t>(length)) != ERR_OK)
        {
            pbuf_free(p);
            return;
        }

        // nif.input is ethernet_input, installed by netif_add(). On
        // success lwIP takes ownership of the pbuf and frees it itself;
        // on failure the pbuf is still ours to release.
        if (nif.input(p, &nif) != ERR_OK)
        {
            pbuf_free(p);
        }
    }
}

template <typename TEth>
void NetIf<TEth>::apply_link_up(const PhySettings& s)
{
    // Re-running start() is what pushes the newly negotiated speed/duplex
    // into MACCR. Safe on an already-running interface -- see net.h.
    if (!eth.start())
    {
        // The MAC would not take the new settings, so the link is not
        // usable. If this was the defensive Up->Up resync (state already
        // Up, PHY renegotiated without a link-down edge), leaving `state`
        // at Up here would mean lwIP keeps passing traffic over a MAC
        // still clocked for the old settings -- corrupting every frame
        // with no fault indication. Force Down instead so both `state`
        // and lwIP's link flag agree the interface is not usable; the
        // next poll() tick retries from a clean Down->Up transition.
        if (state == LinkState::Up)
        {
            apply_link_down();
        }

        return;
    }

    applied_settings = s;
    state = LinkState::Up;

    // Last, and only once the MAC is genuinely ready: this is the edge
    // that releases lwIP's queued traffic and kicks DHCP discovery.
    netif_set_link_up(&nif);
}

template <typename TEth>
void NetIf<TEth>::apply_link_down()
{
    state = LinkState::Down;

    // Tells lwIP to stop handing us frames it cannot send and to suspend
    // the DHCP lease. The MAC deliberately stays enabled -- see net.h.
    netif_set_link_down(&nif);
}

}  // namespace EoT::Net
