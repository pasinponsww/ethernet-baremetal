/**
 * @file gpio.h
 * @author Farhaan Khan
 * @date 
 */

#pragma once
#include <concepts>
#include <cstdint>

namespace EoT
{

/**
     * Concept which forces derived class to have set, get, 
     * and toggle methods.
     */
template <typename T>
concept GpioReq = requires(T t, uint8_t value) {
    {
        t.set(value)
    } -> std::same_as<void>;
    {
        t.get()
    } -> std::same_as<uint8_t>;
    {
        t.toggle()
    } -> std::same_as<void>;
};

template <typename T>
    requires GpioReq<T>
class Gpio
{

public:
    /**
     * @brief constructor which uses a static_assert if normal concept syntax doesn't work
     */
    Gpio()
    {
        //static_assert(GpioReqT> && std::derived_from<T, Gpio>);
    }

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
