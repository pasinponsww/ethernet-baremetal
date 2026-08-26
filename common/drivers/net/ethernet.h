/**
 * @file ethernet.h
 * @brief Generic Ethernet driver CRTP interface
 * @note This is the platform-agnostic contract (init/start/stop,
 *       transmit/receive, link status) that couples the PHY, MAC, DMA, and
 *       MTL layers together behind one frame-level interface. The concrete
 *       implementation is per-platform, same as Mac<T>/Mtl<T> -- see
 *       st_eth.h for the STM32H7 one.
 */

#pragma once
#include <concepts>
#include <cstddef>
#include <span>
#include "eth_mac.h"
#include "lan8742.h"

namespace EoT
{

/**
 * @brief Result of a transmit or receive operation
 */
enum class EthernetStatus : uint8_t
{
    Ok = 0,
    LinkDown,
    NoBuffer,
    Timeout,
    MacError,
    PhyError,
};

// clang-format off
template <typename T>
concept EthernetReq = requires(
    T& eth,
    const T& const_eth,
    const MacAddress& address,
    std::span<const uint8_t> tx_frame,
    std::span<uint8_t> rx_frame,
    size_t& rx_len,
    PhySettings& link_settings)
{
    { eth.init() } -> std::same_as<bool>;
    { eth.set_mac_address(address) } -> std::same_as<bool>;
    { eth.start() } -> std::same_as<bool>;
    { eth.stop() } -> std::same_as<bool>;
    { eth.transmit(tx_frame) } -> std::same_as<EthernetStatus>;
    { eth.receive(rx_frame, rx_len) } -> std::same_as<EthernetStatus>;
    { const_eth.is_link_up() } -> std::same_as<bool>;
    { const_eth.current_link_settings(link_settings) } -> std::same_as<bool>;
};
// clang-format on

template <typename T>
class Ethernet
{
public:

    /**
    * @brief constructor which uses a static_assert as normal concept syntax doesn't work
    */
    Ethernet()
    {
        static_assert(EthernetReq<T> && std::derived_from<T, Ethernet<T>>);
    }

    /**
    * @brief Initialize the PHY, MAC, and DMA descriptor rings
    * @return true if initialization was successful, false otherwise
    */
    bool init()
    {
        return self().init();
    }

    /**
    * @brief Set the MAC address used for perfect filtering
    * @param address The MAC address to set
    * @return true if the MAC address was set successfully, false otherwise
    */
    bool set_mac_address(const MacAddress& address)
    {
        return self().set_mac_address(address);
    }

    /**
    * @brief Bring the link up: apply negotiated/forced PHY settings to the MAC
    *        and enable the transmitter and receiver
    * @return true if the operation was successful, false otherwise
    */
    bool start()
    {
        return self().start();
    }

    /**
    * @brief Disable the transmitter and receiver
    * @return true if the operation was successful, false otherwise
    */
    bool stop()
    {
        return self().stop();
    }

    /**
    * @brief Queue a frame for transmission via the DMA descriptor ring
    * @param frame The frame bytes to transmit (header through payload)
    * @return EthernetStatus::Ok on success, otherwise the reason for failure
    */
    EthernetStatus transmit(std::span<const uint8_t> frame)
    {
        return self().transmit(frame);
    }

    /**
    * @brief Copy the next received frame out of the DMA descriptor ring
    * @param buffer Destination buffer; must be large enough for the frame
    * @param[out] len Filled with the number of bytes written to buffer
    * @return EthernetStatus::Ok on success, otherwise the reason for failure
    */
    EthernetStatus receive(std::span<uint8_t> buffer, size_t& len)
    {
        return self().receive(buffer, len);
    }

    /**
    * @brief Check whether the PHY reports the link as up
    * @return true if the link is up, false otherwise
    */
    bool is_link_up() const
    {
        return self().is_link_up();
    }

    /**
    * @brief Read the PHY's currently negotiated (or forced) speed/duplex
    * @param[out] out Filled with the current link settings on success
    * @return true on success, false if the link is down or the read failed
    */
    bool current_link_settings(PhySettings& out) const
    {
        return self().current_link_settings(out);
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
