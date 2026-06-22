/**
 * @file usart.h
 * @author Farhaan Khan
 */

#pragma once
#include <concepts>
#include <cstdint>
#include <span>

namespace EoT
{

/**
     * Concept which forces derived class to have send and receive methods 
     * 
     */

// clang-format off
template <typename T>
concept UsartReq = requires(T t, const std::span<const uint8_t> data) {
    { t.send(data) } -> std::same_as<bool>;
    { t.receive() } -> std::same_as<bool>;
};
// clang-format on

template <typename T>
class Usart
{
public:
    /**
         * @brief constructor which uses a static_assert as normal concept syntax doesn't work
        */
    Usart()
    {
        static_assert(UsartReq<T> && std::derived_from<T, Usart>);
    }

    bool send(const std::span<const uint8_t> data)
    {
        return self().send(data);
    }

    bool receive()
    {
        return self().receive();
    }

private:
    T& self()
    {
        return static_cast<T&>(*this);
    }
};

}  // namespace EoT