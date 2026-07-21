/**
* @brief Generic MAC Layer template class
* @author Bex Saw
* @date 7/20/2026
* @note This is a generic MAC Layer template class that can be specialized for different platforms.
*/

#pragma once

#include <concepts>
#include <cstdint>

namespace EoT
{

// clang-format off
template <typename T>
concept MacReq = requires(T t)
{
    { t.init() } -> std::same_as<bool>;
};
// clang-format on

template <typename T>
class Mac
{
public:
    /**
    * @brief constructor which uses a static_assert as normal concept syntax doesn't work
    */
    Mac()
    {
        static_assert(MacReq<T> && std::derived_from<T, Mac<T>>);
    }

    /**
    * @brief Initialize the MAC layer
    * @return true if initialization was successful, false otherwise
    */
    bool init()
    {
        return self().init();
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