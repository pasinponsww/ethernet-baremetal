/**
* @file eth_mdio.h
* @brief Ethernet MDIO driver interface
* @author Bex Saw
* @date 7/4/2026
*/

#pragma once

#include <concepts>
#include <cstdint>

namespace EoT
{

enum class EthMdioStatus : uint8_t
{
    Ok = 0,
    NotInitialized,
    InvalidPhyAddr,
    InvalidRegAddr,
    InvalidClock,
    BusyTimeout,
    UnsupportedClause
};

// clang-format off
template <typename T>
concept EthMdioReq = requires(T t,
                              uint8_t phy_addr,
                              uint8_t reg_addr,
                              uint16_t write_data,
                              uint16_t& read_data)
{
    { t.read(phy_addr, reg_addr, read_data) } -> std::same_as<EthMdioStatus>;
    { t.write(phy_addr, reg_addr, write_data) } -> std::same_as<EthMdioStatus>;
};
// clang-format on

template <typename T>
class EthMdio
{
public:
    /**
    * @brief Constructor for the EthMdio class
    * @note This constructor is protected to prevent direct instantiation of the EthMdio class
    */
    EthMdio()
    {
        static_assert(EthMdioReq<T> && std::derived_from<T, EthMdio>);
    }

    /**
     * @brief Read a register from a PHY device
     * @param phy_addr PHY address (0-31)
     * @param reg_addr Register address (0-31)
     * @param data Reference to store the read data
     * @return Status of the read operation
     */
    EthMdioStatus read(uint8_t phy_addr, uint8_t reg_addr, uint16_t& data)
    {
        return self().read(phy_addr, reg_addr, data);
    }

    /**
     * @brief Write a register to a PHY device
     * @param phy_addr PHY address (0-31)
     * @param reg_addr Register address (0-31)
     * @param data Data to write
     * @return Status of the write operation
     */
    EthMdioStatus write(uint8_t phy_addr, uint8_t reg_addr, uint16_t data)
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