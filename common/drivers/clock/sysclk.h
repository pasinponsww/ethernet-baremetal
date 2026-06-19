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
template <typename T>
concept SysclkReq = requires(const T& t) {
    { t.get_freq() } -> std::same_as<uint32_t>;
    { t.set_freq(std::declval<uint32_t>()) } -> std::same_as<void>;
};

class Sysclk
{
    /**
     * @brief Constructor for Sysclk
     * This constructor is protected to prevent direct instantiation of the Sysclk
     * class. It also uses a static assertion to ensure that the derived class
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
        return self.get_freq();
    }

    /**
    * @brief Set the frequency of the user input via BSP
    * @param freq The frequency in Hz
    */
    void set_freq(uint32_t freq)
    {
        self.set_freq(freq);
    }

private:
    /**
    * @brief Helper function to cast this to the derived class
    * @return Reference to the derived class
    */
    T& self()
    {
        return static_cast<T&>(*this);
    }
};
}  // namespace EoT