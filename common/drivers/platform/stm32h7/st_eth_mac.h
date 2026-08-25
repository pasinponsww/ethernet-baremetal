/**
*  @file st_eth_mac.h
*  @brief MAC Layer for STM32H7
*  @author Bex Saw
*  @date 7/20/2026
*/

#pragma once
#include <array>
#include <cstdint>
#include "eth_mac.h"
#include "lan8742.h"
#include "stm32h723xx.h"

namespace EoT
{
namespace StmH7
{

/**
* @brief Settings for the STM32H7 MAC Layer
* @param speed Link speed
* @param duplex Link duplex mode
* @param loopback Loopback mode
* @param mac_address 6-byte MAC address
* @param filter Receive filter configuration
* @param flow_control Flow control configuration
* @param max_frame_size Giant Packet Size Limit (MACECR.GPSL), 0 to leave the
*        default (standard 1518/1522-byte) limit in place
*/
struct StEthMacSettings
{
    PhySpeed speed;
    PhyDuplex duplex;
    bool loopback;
    std::array<uint8_t, 6> mac_address;
    ReceiveFilterConfig filter;
    FlowControlConfig flow_control;
    uint32_t max_frame_size{0U};
};

/**
* @brief Static/global VLAN tag configuration (MACVIR/MACIVIR/MACVTR).
*        Per-packet override is a DMA TX descriptor concern, not this.
*/
struct VlanConfig
{
    bool insert_tag{false};
    uint16_t vlan_id{0U};  // 12-bit VLAN identifier
    uint8_t priority{0U};  // 3-bit 802.1p priority
};

/**
* @brief Wake-on-LAN configuration (MACPCSR, MACRWKPFR)
*/
struct WakeOnLanConfig
{
    bool magic_packet{true};
    bool remote_wakeup{false};
};

/**
* @brief Energy Efficient Ethernet (EEE) configuration (MACCSR, MACLPIR)
*        This structure holds the configuration for the Energy Efficient Ethernet feature.
*        tx_lpi_entry_timer: The time in microseconds before the MAC enters Low Power Idle (LPI) mode after the last frame transmission.
*        tx_lpi_timer: The time in microseconds that the MAC remains in LPI mode before resuming normal operation.
*        rx_lpi_timer: The time in microseconds that the MAC remains in LPI mode before resuming normal operation after receiving a frame.
*/
struct EEEConfig
{
    bool enabled{false};
    uint16_t tx_lpi_entry_timer{0U};
    uint16_t tx_lpi_timer{0U};
    uint16_t rx_lpi_timer{0U};
};

/**
* @brief PTP timestamping configuration (MACTSCR)
* enable: Whether PTP timestamping is enabled or disabled.
* one_step: Whether one-step timestamping is enabled or disabled.
* sub_second_increment: Whether sub-second increment is enabled or disabled.
* sub_second_increment_value: The value to be used for sub-second increment, if enabled.
*/
struct PtpTimestampingConfig
{
    bool enabled{false};
    bool one_step{false};
    bool sub_second_increment{false};
    uint32_t sub_second_increment_value{0U};
};

/**
* @brief Parameters for the STM32H7 MAC Layer
* @param settings MAC settings
* @param base_addr Base address of the ETH peripheral
*/
struct StEthMacParams
{
    StEthMacSettings settings;
    ETH_TypeDef* base_addr;
};

class StEthMac : public Mac<StEthMac>
{
public:
    explicit StEthMac(const StEthMacParams& params);

    /**
    * @brief Initialize the MAC layer
    * @return true if initialization was successful, false otherwise
    */
    bool init();

    /**
    * @brief Program the station MAC address used for perfect-filter matching
    * @param addr 6-byte MAC address
    * @return true if the MAC address was set successfully, false otherwise
    */
    bool set_mac_address(const std::array<uint8_t, 6>& addr);

    /**
    * @brief Apply link settings
    * @param speed Link speed
    * @param duplex Link duplex mode
    * @return true if settings were applied successfully, false otherwise
    */
    bool apply_link_settings(PhySpeed speed, PhyDuplex duplex);

    /**
    * @brief Enable or disable the MAC transmitter (MACCR.TE)
    * @param on true to enable, false to disable
    * @return true if the operation was successful, false otherwise
    */
    bool enable_tx(bool on);

    /**
    * @brief Enable or disable the MAC receiver (MACCR.RE)
    * @param on true to enable, false to disable
    * @return true if the operation was successful, false otherwise
    */
    bool enable_rx(bool on);

    /**
    * @brief Set the loopback mode
    * @param on true to enable loopback, false to disable
    * @return true if the operation was successful, false otherwise
    */
    bool set_loopback(bool on);

    /**
    * @brief Configure destination-address receive filtering (MACPFR)
    * @param config Receive filter configuration
    * @return true if the filter was configured successfully, false otherwise
    */
    bool set_receive_filter(const ReceiveFilterConfig& config);

    /**
    * @brief Enable or disable IP/TCP/UDP checksum offload (MACCR.IPC)
    * @param on true to enable, false to disable
    * @return true if the operation was successful, false otherwise
    */
    bool set_checksum_offload(bool on);

    /**
    * @brief Configure 802.3x flow control / pause frames
    *        (MACTFCR.TFE/PT, MACRFCR.RFE)
    * @param config Flow control configuration
    * @return true if the operation was successful, false otherwise
    */
    bool set_flow_control(const FlowControlConfig& config);

    /**
    * @brief Set the Giant Packet Size Limit (MACECR.GPSL, MACCR.GPSLCE)
    * @param frame_size Max frame size in bytes, must fit in 14 bits (<= 0x3FFF)
    * @return true on success, false if frame_size doesn't fit or on error
    */
    bool set_max_frame_size(uint32_t frame_size);

    /**
    * @brief Whether the MAC transmitter is currently enabled (reads
    *        MACCR.TE directly, not shadow state)
    */
    bool is_transmitter_enabled() const;

    /**
    * @brief Whether the MAC receiver is currently enabled (reads MACCR.RE
    *        directly, not shadow state)
    */
    bool is_receiver_enabled() const;

    /**
    * @brief Whether the MAC's own loopback mode is currently enabled
    */
    bool is_loopback_enabled() const;

    /**
    * @note VLAN, EEE, Wake-on-LAN, and PTP timestamping are feature that 
    *      are introduce for ETHERNET MAC in H7.
    * What it is?
    *   - VLAN - Virtual Local Area Network (allow us to create multiple virtual networks on a single physical network)
    *   - EEE - Energy Efficient Ethernet (reduce power consumption during periods of low data activity)
    *   - Wake-on-LAN - allows a computer to be turned on or awakened by a network message
    *   - PTP - Precision Time Protocol (synchronize clocks throughout a computer network)
    */

    // We have setter/getter and this is handle in init() baked into the settings.

    /**
    * @brief Configure static/global VLAN tag insertion and detection
    *        (MACVIR, MACIVIR, MACVTR)
    * @param config VLAN configuration
    * @return true if the operation was successful, false otherwise
    */
    bool set_vlan_config(const VlanConfig& config);

    /**
    * @brief Enable or disable IEEE 802.3az Energy Efficient Ethernet (LPI)
    *        (MACLCSR, MACLTCR, MACLETR, MAC1USTCR)
    * @param on true to enable, false to disable
    * @return true if the operation was successful, false otherwise
    */
    bool enable_eee(bool on);

    /**
    * @brief Configure Wake-on-LAN (Magic Packet / remote wakeup detection)
    *        (MACPCSR, MACRWKPFR)
    * @param config Wake-on-LAN configuration
    * @return true if the operation was successful, false otherwise
    */
    bool set_wake_on_lan(const WakeOnLanConfig& config);

    /**
    * @brief Enable or disable IEEE 1588/PTPv2 hardware timestamping
    *        (MACTSCR). Per-packet timestamp readback is a DMA descriptor
    *        concern, not this.
    * @param on true to enable, false to disable
    * @return true if the operation was successful, false otherwise
    */
    bool enable_ptp_timestamping(bool on);

    /**
    * @brief Get the current EEE configuration (shadow state)
    * @return The current EEE configuration
    */
    EEEConfig get_eee_config() const;

    /**
    * @brief Get the current Wake-on-LAN configuration (shadow state)
    * @return The current Wake-on-LAN configuration
    */
    WakeOnLanConfig get_wake_on_lan_config() const;

    /**
    * @brief Get the current PTP timestamping configuration (shadow state)
    * @return The current PTP timestamping configuration
    */
    PtpTimestampingConfig get_ptp_timestamping_config() const;

private:
    ETH_TypeDef* base_addr;
    StEthMacSettings settings;
    // No HW shadow for WOL: MACPCSR bits are read back directly, same as TE/RE.
    EEEConfig eee_state_{};
    PtpTimestampingConfig ptp_state_{};
};
}  // namespace StmH7
}  // namespace EoT
