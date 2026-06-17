/**
 * @file gpio.h
 * @author Farhaan Khan
 * @date 
 */

#pragma once

namespace EoT
{
template <typename T>
class Gpio
{

public:

    /**
     * @brief sets a gpio pin to 1 or 0
     * @param 1 or 0
     */
    void set(uint8_t value)
    {
        self().set(value);
    }

    /**
     * @brief reads value of pin
     */
    uint8_t get()
    {
        return self().get();
    }

    /**
     * @brief toggles pin
     */
    void toggle()
    {
        self().toggle();
    }

private:
    T& self()
    {
        return static_cast<T&>(*this);
    }
};
}  // namespace EoT
