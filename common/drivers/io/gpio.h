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

// clang-format off
template <typename T>
concept GpioReq = requires(T t, uint8_t value) {
    { t.set(value) } -> std::same_as<bool>;
    { t.get() } -> std::same_as<bool>;
    { t.toggle() } -> std::same_as<bool>;
};
// clang-format on

template <typename T>
class Gpio
{

public:
    /**
     * @brief constructor which uses a static_assert as normal concept syntax doesn't work
     */
    Gpio()
    {
        static_assert(GpioReq<T> && std::derived_from<T, Gpio>);
    }

    /**
     * @brief sets a gpio pin to 1 or 0
     * @param 1 or 0
     */
    bool set(uint8_t value)
    {
        return self().set(value);
    }

    /**
     * @brief reads value of pin
     */
    bool get() const
    {
        return self().get();
    }

    /**
     * @brief toggles pin
     */
    bool toggle()
    {
        return self().toggle();
    }

private:
    T& self()
    {
        return static_cast<T&>(*this);
    }
};
}  // namespace EoT
