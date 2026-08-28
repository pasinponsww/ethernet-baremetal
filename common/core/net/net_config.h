/**
 * @file net_config.h
 * @brief Configuration for EoT::Net::NetIf
 * @note IP-layer decisions (static vs DHCP, address values) are made by
 *       whoever constructs this struct, not by NetIf itself -- NetIf only
 *       plumbs it through to lwIP. See net.h for the link-state state
 *       machine that owns the parts NetIf does decide.
 */

// This file here handles only the configuration struct and enums, not the NetIf class itself

#pragma once
#include <cstdint>
#include "ethernet.h"       // EoT::MacAddress
#include "lwip/ip4_addr.h"  // provided by lwIP

namespace EoT::Net
{

/**
 * @brief Link-state orchestrator states (see NetIf::poll())
 *       Uninit -> init() not yet called
 *       Fault  -> init() failed, no auto-retry
 *       Down   -> link is down, no IP traffic can flow
 *       Up     -> link is up, IP traffic can flow
 */
enum class LinkState : uint8_t
{
    Uninit = 0,
    Fault,
    Down,
    Up
};

/**
 * @brief How the IPv4 address is obtained
 *      Static -> fixed address, netmask, and gateway are provided in NetConfig
 *      Dhcp   -> address, netmask, and gateway are obtained from a DHCP server
 */
enum class IpMode : uint8_t
{
    Static = 0,
    Dhcp
};

/**
 * @brief Fixed IPv4 configuration, used only when IpMode::Static
 *        address = the device's own IP address
 *        netmask = the subnet mask for the device's network
 *        gateway = the default gateway for the device's network
 */
struct StaticIpConfig
{
    ip4_addr_t address{};
    ip4_addr_t netmask{};
    ip4_addr_t gateway{};
};

// We don't do this process in the BSP because the BSP doesn't know what the IP configs should be
// By embedded inside this file here, it's already in the same namespace as NetIf and can be used to initialize it directly

/**
 * @brief Everything NetIf::init() needs to bring up the Ethernet<T> driver
 *        and the lwIP netif on top of it
 *        mac_address = the device's own MAC address
 *        ip_mode     = how the device obtains its IPv4 address (static or DHCP)
 *        static_ip   = the fixed IPv4 configuration, used only when ip_mode is Static
 *        hostname    = the device's hostname, used only when ip_mode is Dhcp
 *        link_poll_interval_ms = how often NetIf::poll() checks the link state
 */
struct NetConfig
{
    MacAddress mac_address{};
    IpMode ip_mode{IpMode::Dhcp};
    StaticIpConfig static_ip{};
    const char* hostname{"eot-dev"};
    uint32_t link_poll_interval_ms{100};
};

}  // namespace EoT::Net
