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

    // Strip the FCS/pad on receive and let the MAC append it on transmit
    // (MACCR.ACS/CST) -- lwIP's buffers are never given the FCS, so this
    // has to be on unconditionally rather than left as a per-instance
    // setting.
    SetReg(&base_addr->MACCR, 1U, ETH_MACCR_ACS_Pos, 1U);
    SetReg(&base_addr->MACCR, 1U, ETH_MACCR_CST_Pos, 1U);

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

    // Mac Address format: 6 bytes, e.g., 00:11:22:33:44:55

    if (base_addr == nullptr)
    {
        return false;
    }

    // lo and hi are the low and high 32-bit words of the MAC address, respectively.
    const uint32_t lo = (static_cast<uint32_t>(addr[3]) << 24) |
                        (static_cast<uint32_t>(addr[2]) << 16) |
                        (static_cast<uint32_t>(addr[1]) << 8) |
                        static_cast<uint32_t>(addr[0]);
    const uint32_t hi =
        (static_cast<uint32_t>(addr[5]) << 8) | static_cast<uint32_t>(addr[4]);

    // Program the MAC address into the MACA0LR and MACA0HR registers
    // The MACA0HR register's AE bit (bit 31) must be set to enable the address.
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

    // Set the speed and duplex mode in the MACCR register
    // Link Settings for Speed (FES) and Duplex Mode (DM) are in MACCR
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

    // Set the Transmitter Enable (TE) bit in the MACCR register to enable or disable the transmitter
    SetReg(&base_addr->MACCR, on ? 1U : 0U, ETH_MACCR_TE_Pos, 1U);

    return true;
}

bool StEthMac::enable_rx(bool on)
{
    if (base_addr == nullptr)
    {
        return false;
    }

    // Set the Receiver Enable (RE) bit in the MACCR register to enable or disable the receiver
    SetReg(&base_addr->MACCR, on ? 1U : 0U, ETH_MACCR_RE_Pos, 1U);

    return true;
}

bool StEthMac::set_loopback(bool on)
{
    if (base_addr == nullptr)
    {
        return false;
    }

    // Set the Loopback Mode (LM) bit in the MACCR register to enable or disable loopback mode
    SetReg(&base_addr->MACCR, on ? 1U : 0U, ETH_MACCR_LM_Pos, 1U);

    // Update the settings to reflect the new loopback configuration
    settings.loopback = on;

    return true;
}

bool StEthMac::set_receive_filter(const ReceiveFilterConfig& config)
{
    if (base_addr == nullptr)
    {
        return false;
    }

    // Set receive filter settings in the MACPFR register
    // Promiscuous Mode (PR) is in MACPFR, Pass All Multicast (PM) is in MACPFR, and Disable Broadcast Filter (DBF) is in MACPFR

    constexpr uint32_t filter_mask =
        ETH_MACPFR_PR | ETH_MACPFR_PM | ETH_MACPFR_DBF;
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

    // Write the updated filter settings to the MACPFR register
    base_addr->MACPFR = filter;
    // Update the settings to reflect the new receive filter configuration
    settings.filter = config;

    return true;
}

bool StEthMac::set_checksum_offload(bool on)
{
    if (base_addr == nullptr)
    {
        return false;
    }

    // Set Checksum Offload Enable (IPCO) in the MACCR register
    SetReg(&base_addr->MACCR, on ? 1U : 0U, ETH_MACCR_IPC_Pos, 1U);

    return true;
}

bool StEthMac::set_flow_control(const FlowControlConfig& config)
{
    if (base_addr == nullptr)
    {
        return false;
    }

    // Set flow control settings in the MACTFCR and MACRFCR registers
    // Receive Flow Control Enable (RFCE) is in MACRFCR, Transmit Flow Control Enable (TFCE) is in MACTFCR
    SetReg(&base_addr->MACTFCR, config.tx_pause ? 1U : 0U, ETH_MACTFCR_TFE_Pos,
           1U);
    SetReg(&base_addr->MACTFCR, config.pause_time, ETH_MACTFCR_PT_Pos, 16U);
    SetReg(&base_addr->MACRFCR, config.rx_pause ? 1U : 0U, ETH_MACRFCR_RFE_Pos, 1U);

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

    // Set GPSL in the MACCR and MACECR registers to configure the maximum frame size
    // Giant Packet Size Limit Control Enable (GPSLCE) is in MACCR, and GPSL is in MACECR
    SetReg(&base_addr->MACECR, frame_size, ETH_MACECR_GPSL_Pos, 14U);
    SetReg(&base_addr->MACCR, 1U, ETH_MACCR_GPSLCE_Pos, 1U);

    // Update the settings to reflect the new maximum frame size
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

bool StEthMac::set_vlan_config(const VlanConfig& config)
{
    if (base_addr == nullptr)
    {
        return false;
    }

    // VL is a 12-bit VID field, priority a 3-bit field (MACVIR/MACVTR share layout).
    if (config.vlan_id > 0x0FFFU || config.priority > 0x07U)
    {
        return false;
    }

    const uint32_t tag =
        (static_cast<uint32_t>(config.priority) << ETH_MACVIR_VLT_UP_Pos) |
        config.vlan_id;

    // MACVIR: tag applied to transmitted frames.
    SetReg(&base_addr->MACVIR, tag, ETH_MACVIR_VLT_Pos, 16U);
    SetReg(&base_addr->MACVIR, config.insert_tag ? 0x2U : 0x0U,
           ETH_MACVIR_VLC_Pos, 2U);

    // MACVTR: tag used for receive-side VLAN comparison/detection.
    SetReg(&base_addr->MACVTR, tag, ETH_MACVTR_VL_Pos, 16U);
    SetReg(&base_addr->MACVTR, config.insert_tag ? 1U : 0U, ETH_MACVTR_ETV_Pos,
           1U);

    return true;
}

bool StEthMac::enable_eee(bool on)
{
    if (base_addr == nullptr)
    {
        return false;
    }

    // No setter exposes tx_lpi_entry_timer/tx_lpi_timer yet, so whatever is
    // currently in eee_state_ (0 = immediate) is what gets programmed here.
    SetReg(&base_addr->MACLETR, eee_state_.tx_lpi_entry_timer,
           ETH_MACLETR_LPIET_Pos, 20U);
    SetReg(&base_addr->MACLTCR, eee_state_.tx_lpi_timer, ETH_MACLTCR_TWT_Pos,
           16U);

    // rx_lpi_timer has no dedicated register on this MAC -- RX LPI exit is
    // automatic, so it's tracked in the shadow state but never written.
    SetReg(&base_addr->MACLCSR, on ? 1U : 0U, ETH_MACLCSR_LPITXA_Pos, 1U);
    SetReg(&base_addr->MACLCSR, on ? 1U : 0U, ETH_MACLCSR_LPITE_Pos, 1U);
    SetReg(&base_addr->MACLCSR, on ? 1U : 0U, ETH_MACLCSR_LPIEN_Pos, 1U);

    eee_state_.enabled = on;
    return true;
}

bool StEthMac::set_wake_on_lan(const WakeOnLanConfig& config)
{
    if (base_addr == nullptr)
    {
        return false;
    }

    // This is define in the MACPCSR register, but the MACRWKPFR register is read-only and
    // only indicates whether a WOL event has occurred. The MACPCSR bits are read back directly, 
    // so we don't need to maintain shadow state for them.
    SetReg(&base_addr->MACPCSR, config.magic_packet ? 1U : 0U,
           ETH_MACPCSR_MGKPKTEN_Pos, 1U);
    SetReg(&base_addr->MACPCSR, config.remote_wakeup ? 1U : 0U,
           ETH_MACPCSR_RWKPKTEN_Pos, 1U);

    return true;
}

bool StEthMac::enable_ptp_timestamping(bool on)
{
    if (base_addr == nullptr)
    {
        return false;
    }

    // Set the Timestamp Enable (TSENA) bit in the MACTSCR register to enable or disable PTP timestamping
    SetReg(&base_addr->MACTSCR, on ? 1U : 0U, ETH_MACTSCR_TSENA_Pos, 1U);

    ptp_state_.enabled = on;
    return true;
}

EEEConfig StEthMac::get_eee_config() const
{
    return eee_state_;
}

WakeOnLanConfig StEthMac::get_wake_on_lan_config() const
{
    // Read the current WOL configuration from the MACPCSR register
    WakeOnLanConfig config{};

    if (base_addr == nullptr)
    {
        return config;
    }

    // Read the current WOL configuration from the MACPCSR register
    // The MACPCSR register contains the Magic Packet Enable (MGKPKTEN) and Remote Wakeup Enable (RWKPKTEN) bits
    config.magic_packet = (base_addr->MACPCSR & ETH_MACPCSR_MGKPKTEN) != 0U;
    config.remote_wakeup = (base_addr->MACPCSR & ETH_MACPCSR_RWKPKTEN) != 0U;

    return config;
}

PtpTimestampingConfig StEthMac::get_ptp_timestamping_config() const
{
    return ptp_state_;
}

}  // namespace EoT::StmH7
