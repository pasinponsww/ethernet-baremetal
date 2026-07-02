/**
* @file crc.h
* @brief CRC calculation
* @author Bex Sawetrattanathumrong
* @date 6/19/2026
*/

#pragma once
#include <concepts>
#include <cstdint>
#include <span>

namespace EoT
{

// clang-format off
template <typename T>
concept CrcReq =
    requires(T crc,
             std::span<const uint32_t> d32,
             std::span<const uint16_t> d16,
             std::span<const uint8_t> d8,
             uint32_t& result,
             uint32_t expected) {
        { crc.compute(d32, result) } -> std::same_as<bool>;
        { crc.compute(d16, result) } -> std::same_as<bool>;
        { crc.compute(d8, result) } -> std::same_as<bool>;

        { crc.compare(d32, expected) } -> std::same_as<bool>;
        { crc.compare(d16, expected) } -> std::same_as<bool>;
        { crc.compare(d8, expected) } -> std::same_as<bool>;
    };
// clang-format on

template <typename T>
class Crc
{

public:
    /**
    * @brief constructor which uses a static_assert as normal concept syntax doesn't work
    */
    Crc()
    {
        static_assert(CrcReq<T> && std::derived_from<T, Crc<T>>);
    }

    /**
    * @brief Compute checksum from data
    *           Data bits size:   32 | 8, 16, 32 
    *           Poly bits size:   32 | 7, 8, 16, 32     
    * 
    * @param data block of memory to compute crc on
    * @param result block of memory to store result in
    * @return true if successful, false otherwise
    */
    bool compute(std::span<const uint32_t> data, uint32_t& result)
    {
        return self().compute(data, result);
    }

    bool compute(std::span<const uint16_t> data, uint32_t& result)
    {
        return self().compute(data, result);
    }

    bool compute(std::span<const uint8_t> data, uint32_t& result)
    {
        return self().compute(data, result);
    }

    /**
    * @brief Compare checksum from data to expected value
    * @param data block of memory to compute crc on
    * @param expected expected crc value
    */
    bool compare(std::span<const uint32_t> data, uint32_t expected)
    {
        return self().compare(data, expected);
    }

    bool compare(std::span<const uint16_t> data, uint32_t expected)
    {
        return self().compare(data, expected);
    }

    bool compare(std::span<const uint8_t> data, uint32_t expected)
    {
        return self().compare(data, expected);
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