/**
* @brief Generic MAC Layer template class
* @author Bex Saw
* @date 7/20/2026
* @note This is a generic MAC Layer template class that can be specialized for different platforms.
*/

#pragma once

#include <array>
#include <concepts>
#include <cstdint>
#include "lan8742.h"

/// NOTE: The MAC-LAYER here responsbile for provides an interface for initializing and configuring the MAC, as well as sending and receiving Ethernet frames.
// It is not responsible for PHY management, which is handled by the PHY-LAYER already (lan8742.h).

namespace EoT
{

// clang-format off
template <typename T>
concept MacReq = requires(T t,
                          const std::array<uint8_t, 6>& addr,
                          PhySpeed speed,
                          PhyDuplex duplex,
                          bool on)
{
    { t.init() } -> std::same_as<bool>;
    { t.set_mac_address(addr) } -> std::same_as<bool>;
    { t.apply_link_settings(speed, duplex) } -> std::same_as<bool>;
    { t.enable(on) } -> std::same_as<bool>;
    { t.set_loopback(on) } -> std::same_as<bool>;
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
    * @brief Initialize the MAC layer
    * @return true if initialization was successful, false otherwise
    */
    bool init()
    {
        return self().init();
    }

    /**
    * @brief Program the station MAC address used for perfect-filter matching
    * @param addr 6-byte MAC address
    * @return true on success, false otherwise
    */
    bool set_mac_address(const std::array<uint8_t, 6>& addr)
    {
        return self().set_mac_address(addr);
    }

    /**
    * @brief Apply the negotiated speed/duplex reported by the PHY to the MAC
    * @param speed Link speed reported by the PHY
    * @param duplex Link duplex reported by the PHY
    * @return true on success, false otherwise
    */
    bool apply_link_settings(PhySpeed speed, PhyDuplex duplex)
    {
        return self().apply_link_settings(speed, duplex);
    }

    /**
    * @brief Enable or disable the MAC transmitter/receiver
    * @param on true to enable, false to disable
    * @return true on success, false otherwise
    */
    bool enable(bool on)
    {
        return self().enable(on);
    }

    /**
    * @brief Enable or disable the MAC's own loopback mode. This is separate
    *        from any PHY-side loopback -- it exercises the DMA/MTL/MAC
    *        datapath without needing a PHY present at all.
    * @param on true to enable loopback, false for normal operation
    * @return true on success, false otherwise
    */
    bool set_loopback(bool on)
    {
        return self().set_loopback(on);
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