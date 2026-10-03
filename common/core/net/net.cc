/**
 * @file net.cc
 * @brief Non-template part of the net layer
 * @note NetIf<TEth> is a template, so its bodies live in net_impl.h where
 *       they are visible at every instantiation point -- none of it is
 *       compiled here. What is left for this translation unit is the
 *       handful of ordinary functions that do not depend on TEth: the
 *       ip4_addr_t helpers declared in net_config.h.
 *
 * @author Bex Saw
 */

#include "net.h"

#include <cstring>  // std::strlen, to measure what ip4addr_ntoa_r wrote

namespace EoT::Net
{

ip4_addr_t make_ip4(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
    ip4_addr_t address{};

    // IP4_ADDR packs the octets and applies the host-to-network swap, so
    // this stays correct if the port is ever moved to a big-endian core.
    IP4_ADDR(&address, a, b, c, d);

    return address;
}

size_t format_ip4(const ip4_addr_t& address, char* out, size_t capacity)
{
    // Worst case "255.255.255.255" is 15 characters plus a terminator.
    constexpr size_t kMaxTextLength{16U};

    if (out == nullptr || capacity < kMaxTextLength)
    {
        return 0U;
    }

    // lwIP already ships this exact algorithm (digit division, no stdio)
    // as the reentrant ip4addr_ntoa_r; the non-reentrant ip4addr_ntoa()
    // this function was written to avoid -- the one with the shared
    // static buffer -- is a different function entirely.
    if (ip4addr_ntoa_r(&address, out, static_cast<int>(capacity)) == nullptr)
    {
        return 0U;
    }

    return std::strlen(out);
}

}  // namespace EoT::Net
