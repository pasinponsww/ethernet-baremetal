/**
* @file sysclk.h
* @brief System clock configuration
* @author Bex Sawetrattanathumrong
* @date 6/18/2026
*/

#pragma once
#include <concepts>

namespace EoT
{

/**
* @brief Concept for system clock requirements
* This concept requires that the type T has the following member functions:
* - `uint32_t get_freq() const`: A function that returns the current frequency of
*   the system clock in Hz.
* - `void set_freq(uint32_t freq)`: A function that sets the frequency of
*   the system clock to the specified value in Hz.
*/

// clang-format off
template <typename T>
concept SysclkReq = requires(const T& t) {
    { t.get_freq() } -> std::same_as<uint32_t>;
};

// clang-format on
template <typename T>
class Sysclk
{
public:
    /**
     * @brief Constructor for Sysclk
     * It uses a static assertion to ensure that the derived class
     * meets the requirements of the SysclkReq concept.
     */
    Sysclk()
    {
        static_assert(SysclkReq<T> && std::derived_from<T, Sysclk>);
    }

    /**
    * @brief Get the frequency of the user input via BSP
    * @return The frequency in Hz
    */
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