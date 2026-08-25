/**
 * @file eth_mac.h
 * @brief Generic Ethernet MAC CRTP interface
 * @author Bex Saw
 * @date 7/20/2026
 */

#pragma once
#include <array>
#include <concepts>
#include <cstdint>
#include "lan8742.h"

namespace EoT
{

using MacAddress = std::array<uint8_t, 6>;

/**
 * @brief Configuration for the receive filter
 * @details This structure defines the settings for filtering incoming frames.
 */
struct ReceiveFilterConfig
{
    bool promiscuous{false};
    bool accept_all_multicast{false};
    bool accept_broadcast{true};
};

/**
 * @brief Configuration for flow control
 * @details This structure defines the settings for managing flow control in the Ethernet MAC.
 */
struct FlowControlConfig
{
    bool tx_pause{false};
    bool rx_pause{false};
    uint16_t pause_time{0xFFFF};
};

// clang-format off
template <typename T>
concept MacReq = requires(
    T& mac,
    const T& const_mac,
    const MacAddress& address,
    PhySpeed speed,
    PhyDuplex duplex,
    const ReceiveFilterConfig& filter,
    const FlowControlConfig& flow_control,
    uint32_t frame_size,
    bool enabled)
{
    { mac.init() } -> std::same_as<bool>;
    { mac.set_mac_address(address) } -> std::same_as<bool>;
    { mac.apply_link_settings(speed, duplex) } -> std::same_as<bool>;
    { mac.enable_tx(enabled) } -> std::same_as<bool>;
    { mac.enable_rx(enabled) } -> std::same_as<bool>;
    { mac.set_loopback(enabled) } -> std::same_as<bool>;
    { mac.set_receive_filter(filter) } -> std::same_as<bool>;
    { mac.set_checksum_offload(enabled) } -> std::same_as<bool>;
    { mac.set_flow_control(flow_control) } -> std::same_as<bool>;
    { mac.set_max_frame_size(frame_size) } -> std::same_as<bool>;
    { const_mac.is_transmitter_enabled() } -> std::same_as<bool>;
    { const_mac.is_receiver_enabled() } -> std::same_as<bool>;
    { const_mac.is_loopback_enabled() } -> std::same_as<bool>;
};
// clang-format on

template <typename T>
class Mac
{
public:
    /**
    * @brief constructor which uses a static_assert as normal concept syntax doesn't work
    */
    Mac()
    {
        static_assert(MacReq<T> && std::derived_from<T, Mac<T>>);
    }

    /**
    * @brief Initialize the MAC
    * @return true if initialization was successful, false otherwise
    */
    bool init()
    {
        return self().init();
    }

    /**
    * @brief Apply link settings (speed and duplex)
    * @param speed The link speed to set
    * @param duplex The duplex mode to set
    * @return true if the settings were applied successfully, false otherwise
    */
    bool apply_link_settings(PhySpeed speed, PhyDuplex duplex)
    {
        return self().apply_link_settings(speed, duplex);
    }

    /**
    * @brief Enable or disable the transmitter
    * @param enabled true to enable the transmitter, false to disable it
    * @return true if the operation was successful, false otherwise
    */
    bool enable_tx(bool enabled)
    {
        return self().enable_tx(enabled);
    }

    /**
    * @brief Enable or disable the receiver
    * @param enabled true to enable the receiver, false to disable it
    * @return true if the operation was successful, false otherwise
    */
    bool enable_rx(bool enabled)
    {
        return self().enable_rx(enabled);
    }

    /**
    * @brief Enable or disable the MAC
    * @param enabled true to enable the MAC, false to disable it
    * @return true if the operation was successful, false otherwise
    */
    bool enable(bool enabled)
    {
        if (enabled)
        {
            if (!self().enable_rx(true))
            {
                return false;
            }

            return self().enable_tx(true);
        }

        if (!self().enable_tx(false))
        {
            return false;
        }

        return self().enable_rx(false);
    }

    /*                                                                */
    /*                      Setter Functions                          */
    /*                                                                */

    /**
    * @brief Set the loopback mode
    * @param enabled true to enable loopback, false to disable it
    * @return true if the operation was successful, false otherwise
    */
    bool set_loopback(bool enabled)
    {
        return self().set_loopback(enabled);
    }

    /**
    * @brief Set the receive filter
    * @param config The receive filter configuration
    * @return true if the operation was successful, false otherwise
    */
    bool set_receive_filter(const ReceiveFilterConfig& config)
    {
        return self().set_receive_filter(config);
    }

    /**
    * @brief Set the checksum offload
    * @param enabled true to enable checksum offload, false to disable it
    * @return true if the operation was successful, false otherwise
    */
    bool set_checksum_offload(bool enabled)
    {
        return self().set_checksum_offload(enabled);
    }

    /**
    * @brief Set the flow control
    * @param config The flow control configuration
    * @return true if the operation was successful, false otherwise
    */
    bool set_flow_control(const FlowControlConfig& config)
    {
        return self().set_flow_control(config);
    }

    /**
    * @brief Set the maximum frame size
    * @param frame_size The maximum frame size
    * @return true if the operation was successful, false otherwise
    */
    bool set_max_frame_size(uint32_t frame_size)
    {
        return self().set_max_frame_size(frame_size);
    }

    /**
    * @brief Set the MAC address for perfect filtering
    * @param address The MAC address to set
    * @return true if the MAC address was set successfully, false otherwise
    */
    bool set_mac_address(const MacAddress& address)
    {
        return self().set_mac_address(address);
    }

    /*                                                                */
    /*                      Checker Health Functions                  */
    /*                                                                */

    /**
    * @brief Check if the transmitter is enabled
    * @return true if the transmitter is enabled, false otherwise
    */
    bool is_transmitter_enabled() const
    {
        return self().is_transmitter_enabled();
    }

    /**
    * @brief Check if the receiver is enabled
    * @return true if the receiver is enabled, false otherwise
    */
    bool is_receiver_enabled() const
    {
        return self().is_receiver_enabled();
    }

    /**
    * @brief Check if the MAC is enabled
    * @return true if the MAC is enabled, false otherwise
    */
    bool is_enabled() const
    {
        return is_transmitter_enabled() || is_receiver_enabled();
    }

    /**
    * @brief Check if the loopback is enabled
    * @return true if the loopback is enabled, false otherwise
    */
    bool is_loopback_enabled() const
    {
        return self().is_loopback_enabled();
    }

private:
    T& self()
    {
        return static_cast<T&>(*this);
    }

    const T& self() const
    {
        return static_cast<const T&>(*this);
    }
};

}  // namespace EoT