/**
 * @file dma.h
 * @brief Ethernet DMA driver interface
 * @author Farhaan Khan
 * @date 7/14/2026
 */

#pragma once
#include <concepts>
#include <cstdint>

namespace EoT
{

// clang-format off
template <typename T>
concept EthDmaReq = requires(T t)
{
    { t.init() } -> std::same_as<bool>;
};

// clang-format on

template <typename T>
class EthDma
{
public:
    /**
    * @brief Constructor for the EthDma class
    * @note This constructor is protected to prevent direct instantiation of the EthDma class
    */
    EthDma()
    {
        static_assert(EthDmaReq<T> && std::derived_from<T, EthDma>);
    }

    /**
    * @brief Initialize Ethernet DMA
    * @return status of initialize operation
    */
    bool init()
    {
        return self().init();
    }

    // TODO: Some send function
    // TODO: Some receive function

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