/**
 * @file net.h
 * @brief Link-state orchestrator + lwIP netif glue
 * @note This is NOT a network-layer state machine - IP addressing, ARP,
 *       DHCP lease lifecycle, TCP/UDP all run inside lwIP itself, driven by
 *       sys_check_timeouts() from poll(). This class owns exactly two
 *       things: 
 *       (1) periodically sampling the PHY via Ethernet<T> and
 *       reacting to link up/down/renegotiation by resyncing the MAC and
 *       lwIP's netif link flag
 *       (2) the low_level_output/input glue a
 *       lwIP netif needs to actually move frames through Ethernet<T>.
 *
 *       No RTOS, no mutex/semaphore/thread - lwIP runs in NO_SYS=1 mode,
 *       driven entirely from poll() called every superloop tick.
 *
 *       NetIf<TEth> is a plain template class (not CRTP - nothing derives
 *       from it), generic over any Ethernet<T>-conforming type, holding it
 *       by reference the same way Board<THw> holds its peripherals.
 *
 *       Data flow, and where each hop is implemented:
 *
 *         Cable -> PHY -> MAC -> DMA -> RAM -> net.h/.cc -> pbuf -> IP -> TCP/UDP -> application
 *                  |------ EoT::Ethernet<T> ------|  |-- NetIf<TEth> --|  |----- lwIP ----------|
 *                  (lan8742.h, st_eth_mac.*,          (this file,        (external/lightweight-ip,
 *                   st_eth_mtl.*, st_eth.* --          low_level_output/   NO_SYS=1, driven by
 *                   MTL/MAC/DMA registers)              service_rx here    poll()'s
 *                                                        turn frames into  sys_check_timeouts())
 *                                                        pbuf, and back)
 *
 * @author Bex Saw
 */

#pragma once
#include <array>
#include <cstdint>
#include "ethernet.h"  // EoT::Ethernet<T>, EoT::PhySettings, EoT::EthernetStatus
#include "lwip/err.h"  // lwIP error codes, used in low_level_init/output
#include "lwip/netif.h"  // lwIP netif type, used in NetIf<TEth> and low_level_init/output
#include "lwip/pbuf.h"  // pbuf is the lwIP packet buffer type, used in low_level_output/input
#include "net_config.h"  // EoT::NetConfig, EoT::LinkState, EoT::IpMode, EoT::StaticIpConfig

namespace EoT::Net
{

template <typename TEth>
class NetIf
{
public:
    explicit NetIf(EoT::Ethernet<TEth>& eth) : eth(eth)
    {
    }

    /**
     * @brief One-shot bring-up of the Ethernet<T> driver and the lwIP netif
     * @note Order: eth.init() -> eth.set_mac_address() -> eth.start(),
     *       bailing to LinkState::Fault on first failure. Then
     *       netif_add(&nif, ..., this, &low_level_init, &ethernet_input)
     *       (ethernet_input, not ip4_input/raw input -- needed so ARP
     *       frames get demuxed before reaching the IP layer). Then branch
     *       on config.ip_mode: dhcp_start(&nif) vs netif_set_addr(&nif,
     *       ...). Finally cache config and run one unforced link sample
     *       (the same logic poll() runs on its rate-limited tick) so a
     *       cable that's already plugged in at boot is caught immediately
     *       instead of waiting for the first poll().
     * @param config MAC address, IP mode/address, hostname, poll interval
     * @return true if every bring-up step succeeded
     */
    bool init(const NetConfig& config);

    /**
     * @brief Service the interface - call every superloop tick
     * @note Always (every call, unconditional):
     *         - service_rx() to drain any received frames into lwIP
     *         - sys_check_timeouts() so lwIP's own ARP/DHCP/TCP timers run
     *       Rate-limited to config.link_poll_interval_ms (via
     *       Utils::get_ms_ticks()):
     *         - sample eth.current_link_settings() and drive the link
     *           state machine:
     *             Down, link now up            -> apply_link_up(sampled)
     *             Up,   link now down           -> apply_link_down()
     *             Up,   still up but settings
     *                   changed (defensive)     -> apply_link_up(sampled)
     *             Down, still down / Up, still
     *                   up & unchanged          -> no-op
     *       Fault/Uninit: poll() is a no-op (no auto-retry of init()).
     */
    void poll();

    LinkState link_state() const
    {
        return state;
    }

    bool is_link_up() const
    {
        return state == LinkState::Up;
    }

    /**
     * @brief Last PhySettings applied to the MAC
     * @return false if the link isn't currently up (out left unmodified)
     */
    bool current_applied_settings(PhySettings& out) const;

    /**
     * @brief Escape hatch for app code, e.g. to print the DHCP-assigned
     *        address once bound
     */
    ::netif& raw_netif()
    {
        return nif;
    }

private:
    /**
     * @brief netif_add() init callback -- lwIP-side field setup ONLY
     * @note hwaddr/hwaddr_len/mtu/flags, netif->output = etharp_output,
     *       netif->linkoutput = &low_level_output. Do NOT do hardware
     *       bring-up here -- that already happened in init() before
     *       netif_add() was called; n->state is `this`, set by init().
     */
    static err_t low_level_init(::netif* n);

    /**
     * @brief lwIP -> Ethernet<T> transmit path
     * @note pbuf_copy_partial(p, tx_buf.data(), p->tot_len, 0) to linearize
     *       the (possibly chained) pbuf into tx_buf -- Ethernet<T>::transmit
     *       takes one contiguous span, no scatter-gather at that layer.
     *       Then eth.transmit(span) and map EthernetStatus -> err_t:
     *         Ok                          -> ERR_OK
     *         NoBuffer                    -> ERR_MEM
     *         LinkDown/MacError/PhyError  -> ERR_IF
     *         Timeout                     -> ERR_TIMEOUT
     */
    static err_t low_level_output(::netif* n, pbuf* p);

    /**
     * @brief Ethernet<T> -> lwIP receive path, drains everything available
     * @note Loop: eth.receive(rx_buf, len); if nothing came back, return.
     *       pbuf_alloc(PBUF_RAW, len, PBUF_POOL), pbuf_take() the bytes in,
     *       nif.input(p, &nif); pbuf_free(p) on failure. Against the mock
     *       DMA this is fully wired but permanently inert (receive()
     *       always reports nothing) -- starts working once DMA is real,
     *       with no changes needed here.
     */
    void service_rx();

    /**
     * @brief Down->Up (or defensive Up->Up resync) action
     * @note eth.start() again -- confirmed safe/idempotent to call
     *       repeatedly (StEthernet::start() re-applies MAC speed/duplex
     *       and re-enables tx/rx without forcing a fresh negotiation or
     *       glitching an already-good link). Then applied_settings = s,
     *       netif_set_link_up(&nif), state = LinkState::Up.
     */
    void apply_link_up(const PhySettings& s);

    /**
     * @brief Up->Down action
     * @note netif_set_link_down(&nif); state = LinkState::Down. Do NOT
     *       call eth.stop() here -- MAC enable/disable stays a separate,
     *       explicit app-level action, not something link state toggles.
     */
    void apply_link_down();

    EoT::Ethernet<TEth>& eth;
    ::netif nif{};
    NetConfig config{};
    LinkState state{LinkState::Uninit};
    PhySettings applied_settings{};
    uint32_t last_link_poll_ms{0};
    std::array<uint8_t, 1518> tx_buf{};
};

}  // namespace EoT::Net

#include "net_impl.h"
