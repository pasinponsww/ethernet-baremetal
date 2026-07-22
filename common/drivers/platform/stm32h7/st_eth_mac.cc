#include "st_eth_mac.h"
#include "reg_helpers.h"

namespace EoT::StmH7
{
StEthMac::StEthMac(const StEthMacParams& params)
    : base_addr(params.base_addr), settings(params.settings)
{
}

bool StEthMac::init()
{
    if (base_addr == nullptr)
    {
        return false;
    }

    // Program the address filter and the initial speed/duplex/loopback/
    // filter/flow-control/frame-size from construction. TE/RE are left
    // disabled here on purpose -- the DMA rings (owned separately) need to
    // be set up before frames can safely flow, so enabling TX/RX is
    // deferred to explicit enable_tx()/enable_rx() calls.
    if (!set_mac_address(settings.mac_address))
    {
        return false;
    }

    if (!apply_link_settings(settings.speed, settings.duplex))
    {
        return false;
    }

    if (!set_loopback(settings.loopback))
    {
        return false;
    }

    if (!set_receive_filter(settings.filter))
    {
        return false;
    }

    if (!set_flow_control(settings.flow_control))
    {
        return false;
    }

    if (settings.max_frame_size != 0U)
    {
        return set_max_frame_size(settings.max_frame_size);
    }

    return true;
}

bool StEthMac::set_mac_address(const std::array<uint8_t, 6>& addr)
{
    if (base_addr == nullptr)
    {
        return false;
    }

    const uint32_t lo = (static_cast<uint32_t>(addr[3]) << 24) |
                        (static_cast<uint32_t>(addr[2]) << 16) |
                        (static_cast<uint32_t>(addr[1]) << 8) |
                        static_cast<uint32_t>(addr[0]);
    const uint32_t hi = (static_cast<uint32_t>(addr[5]) << 8) |
                        static_cast<uint32_t>(addr[4]);

    base_addr->MACA0LR = lo;
    base_addr->MACA0HR = hi | ETH_MACA0HR_AE;

    settings.mac_address = addr;

    return true;
}

bool StEthMac::apply_link_settings(PhySpeed speed, PhyDuplex duplex)
{
    if (base_addr == nullptr)
    {
        return false;
    }

    SetReg(&base_addr->MACCR, speed == PhySpeed::Speed100M ? 1U : 0U,
           ETH_MACCR_FES_Pos, 1U);
    SetReg(&base_addr->MACCR, duplex == PhyDuplex::Full ? 1U : 0U,
           ETH_MACCR_DM_Pos, 1U);

    settings.speed = speed;
    settings.duplex = duplex;
    return true;
}

bool StEthMac::enable_tx(bool on)
{
    if (base_addr == nullptr)
    {
        return false;
    }

    SetReg(&base_addr->MACCR, on ? 1U : 0U, ETH_MACCR_TE_Pos, 1U);
    return true;
}

bool StEthMac::enable_rx(bool on)
{
    if (base_addr == nullptr)
    {
        return false;
    }

    SetReg(&base_addr->MACCR, on ? 1U : 0U, ETH_MACCR_RE_Pos, 1U);
    return true;
}

bool StEthMac::set_loopback(bool on)
{
    if (base_addr == nullptr)
    {
        return false;
    }

    SetReg(&base_addr->MACCR, on ? 1U : 0U, ETH_MACCR_LM_Pos, 1U);
    settings.loopback = on;
    return true;
}

bool StEthMac::set_receive_filter(const ReceiveFilterConfig& config)
{
    if (base_addr == nullptr)
    {
        return false;
    }

    constexpr uint32_t filter_mask = ETH_MACPFR_PR | ETH_MACPFR_PM | ETH_MACPFR_DBF;
    uint32_t filter = base_addr->MACPFR & ~filter_mask;
    if (config.promiscuous)
    {
        filter |= ETH_MACPFR_PR;
    }
    if (config.accept_all_multicast)
    {
        filter |= ETH_MACPFR_PM;
    }
    if (!config.accept_broadcast)
    {
        filter |= ETH_MACPFR_DBF;
    }
    base_addr->MACPFR = filter;

    settings.filter = config;
    return true;
}

bool StEthMac::set_checksum_offload(bool on)
{
    if (base_addr == nullptr)
    {
        return false;
    }

    SetReg(&base_addr->MACCR, on ? 1U : 0U, ETH_MACCR_IPC_Pos, 1U);
    return true;
}

bool StEthMac::set_flow_control(const FlowControlConfig& config)
{
    if (base_addr == nullptr)
    {
        return false;
    }

    SetReg(&base_addr->MACTFCR, config.tx_pause ? 1U : 0U, ETH_MACTFCR_TFE_Pos,
           1U);
    SetReg(&base_addr->MACTFCR, config.pause_time, ETH_MACTFCR_PT_Pos, 16U);
    SetReg(&base_addr->MACRFCR, config.rx_pause ? 1U : 0U, ETH_MACRFCR_RFE_Pos,
           1U);

    settings.flow_control = config;
    return true;
}

bool StEthMac::set_max_frame_size(uint32_t frame_size)
{
    if (base_addr == nullptr)
    {
        return false;
    }

    // GPSL is a 14-bit field
    if (frame_size > 0x3FFFU)
    {
        return false;
    }

    SetReg(&base_addr->MACECR, frame_size, ETH_MACECR_GPSL_Pos, 14U);
    SetReg(&base_addr->MACCR, 1U, ETH_MACCR_GPSLCE_Pos, 1U);

    settings.max_frame_size = frame_size;
    return true;
}

bool StEthMac::is_transmitter_enabled() const
{
    if (base_addr == nullptr)
    {
        return false;
    }

    return (base_addr->MACCR & ETH_MACCR_TE) != 0U;
}

bool StEthMac::is_receiver_enabled() const
{
    if (base_addr == nullptr)
    {
        return false;
    }

    return (base_addr->MACCR & ETH_MACCR_RE) != 0U;
}

bool StEthMac::is_loopback_enabled() const
{
    return settings.loopback;
}

}
