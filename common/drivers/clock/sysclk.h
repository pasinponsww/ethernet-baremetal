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

template <typename T>
concept SysclkReq = requires(const T& t) {
    { t.get_freq() } -> std::same_as<uint32_t>;
    { t.set_freq(std::declval<uint32_t>()) } -> std::same_as<void>;
};

class Sysclk
{
    explicit Sysclk(T clock) : clock(clock)
    {
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