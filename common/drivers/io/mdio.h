/**
* @file mdio.h
* @brief MDIO driver interface
* @author Bex Saw
* @date 7/4/2026
*/

#pragma once

#include <concepts>
#include <cstdint>

namespace EoT
{

enum class MdioStatus : uint8_t
{
    OK = 0,
    BUSY,
    TIMEOUT,
    INVALID_PHY,
    INVALID_REG,
    HARDWARE_ERROR,
};

// clang-format off
template <typename T>
concept MdioReq = requires(T t,
                           uint8_t phy,
                           uint8_t reg,
                           uint16_t data,
                           uint16_t& out)
{
    { t.init() } -> std::same_as<MdioStatus>;
    { t.read(phy, reg, out) } -> std::same_as<MdioStatus>;
    { t.write(phy, reg, data) } -> std::same_as<MdioStatus>;
};
// clang-format on

template <typename T>
class Mdio
{
public:
    /**
    * @brief constructor which uses a static_assert as normal concept syntax doesn't work
    */
    Mdio()
    {
        static_assert(MdioReq<T> && std::derived_from<T, Mdio<T>>);
    }

    /**
    * @brief Read a register from the MDIO interface
    * @param phy_addr The PHY address
    * @param reg_addr The register address
    * @param data Reference to store the read data
    * @return MdioStatus indicating the result of the read operation
    */
    MdioStatus read(uint8_t phy_addr, uint8_t reg_addr, uint16_t& data)
    {
        return self().read(phy_addr, reg_addr, data);
    }

    /**
    * @brief Write a register to the MDIO interface
    * @param phy_addr The PHY address
    * @param reg_addr The register address
    * @param data The data to write
    * @return MdioStatus indicating the result of the write operation
    */
    MdioStatus write(uint8_t phy_addr, uint8_t reg_addr, uint16_t data)
    {
        return self().write(phy_addr, reg_addr, data);
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