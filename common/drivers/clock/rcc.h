/**
* @file rcc.h
* @brief RCC (Reset and Clock Control) driver interface
* @author Bex Sawetrattanathumrong
* @date 7/1/2026
*/

#pragma once
#include <concepts>

namespace EoT
{

/**
* @note This is all I need for the clock driver, assuming for Ethernet as the main peripheral. And uart for debugging.
*/

// clang-format off
template <typename T>
concept RccReq = requires(const T& t) {
    { t.enable_eth_clock(); } -> std::same_as<bool>,
    { t.enable_gpio_clock(); } -> std::same_as<bool>,
    { t.enable_uart_clock(); } -> std::same_as<bool>,
    { t.enable_dma_clock(); } -> std::same_as<bool>,
    { t.enable_crc_clock(); } -> std::same_as<bool>,
    { t.get_freq() } -> std::same_as<uint32_t>;
};

// clang-format on
template <typename T>
class Rcc
{
public:
    /**
     * @brief Constructor for Rcc
     * It uses a static assertion to ensure that the derived class
     * meets the requirements of the RccReq concept.
     */
    Rcc()
    {
        static_assert(RccReq<T> && std::derived_from<T, Rcc>);
    }

    bool enable_eth_clock()
    {
        return self().enable_eth_clock();
    }

    bool enable_gpio_clock()
    {
        return self().enable_gpio_clock();
    }

    bool enable_uart_clock()
    {
        return self().enable_uart_clock();
    }

    bool enable_dma_clock()
    {
        return self().enable_dma_clock();
    }

    bool enable_crc_clock()
    {
        return self().enable_crc_clock();
    }

    uint32_t get_freq() const
    {
        return self().get_freq();
    }

private:
    /**
    * @brief Helper function to cast this to the derived class
    * @return Reference to the derived class
    */
    const T& self() const
    {
        return static_cast<const T&>(*this);
    }
};
}  // namespace EoT