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

    // ip4_addr1..4 read the octets back out in dotted-quad order,
    // undoing the network byte order make_ip4() put them in.
    const uint8_t octets[4] = {
        ip4_addr1(&address),
        ip4_addr2(&address),
        ip4_addr3(&address),
        ip4_addr4(&address),
    };

    size_t written{0U};

    for (size_t i = 0U; i < 4U; ++i)
    {
        if (i != 0U)
        {
            out[written++] = '.';
        }

        // No stdio in this build, so emit the decimal digits by hand.
        // Each octet is 1-3 digits; peel the leading zeros off rather
        // than printing a fixed three.
        const uint8_t value = octets[i];

        if (value >= 100U)
        {
            out[written++] = static_cast<char>('0' + (value / 100U));
        }

        if (value >= 10U)
        {
            out[written++] = static_cast<char>('0' + ((value / 10U) % 10U));
        }

        out[written++] = static_cast<char>('0' + (value % 10U));
    }

    out[written] = '\0';

    return written;
}

}  // namespace EoT::Net
