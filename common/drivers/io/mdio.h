/**
* @file mdio.h
* @brief MDIO driver interface
* @author Bex Saw
* @date 7/4/2026
*/

#pragma once 
#include <concepts>


/**
* @note This is the MDIO interface (Management Data Input/Output) which is used to communicate with PHY devices over the MDIO bus.
* It defines the required methods for reading and writing to PHY registers.
*/

namespace EoT
{

// clang-format off
template <typename T>
concept MdioReq = requires(T t, uint8_t phy, uint8_t reg, uint16_t data) {
    { t.read(phy, reg) } -> std::same_as<bool>;
    { t.write(phy, reg, data) } -> std::same_as<bool>;
};
// clang-format on

template <typename T>
class Mdio
{
public:

    bool read(uint8_t phy_addr, uint8_t reg_addr)
    {
        return self().read(phy_addr, reg_addr);
    }

    bool write(uint8_t phy_addr, uint8_t reg_addr, uint16_t data)
    {
        return self().write(phy_addr, reg_addr, data);
    }

private:
    T& self()
    {
        return static_cast<T&>(*this);
    }
};
}