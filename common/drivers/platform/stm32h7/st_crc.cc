#include "st_crc.h"

namespace EoT::StmH7
{

/** @brief This is a constant for the bit width of the CRC control register's reverse output bit */
static constexpr uint8_t kCRC_CR_REV_OUT_BitWidth{1};
static constexpr uint8_t kCRC_CR_REV_IN_BitWidth{2};
static constexpr uint8_t kCRC_CR_POLYSIZE_BitWidth{2};

/** @brief Loads a value into the CRC data register */

inline void StCrc::load(uint8_t value)
{
    volatile uint8_t* dr8 = reinterpret_cast<volatile uint8_t*>(&crc->DR);
    *dr8 = value;
}
inline void StCrc::load(uint16_t value)
{
    volatile uint16_t* dr16 = reinterpret_cast<volatile uint16_t*>(&crc->DR);
    *dr16 = value;
}
inline void StCrc::load(uint32_t value)
{
    volatile uint32_t* dr32 = reinterpret_cast<volatile uint32_t*>(&crc->DR);
    *dr32 = value;
}

StCrc::StCrc(CRC_TypeDef* const crc, uint32_t initial_crc,
             uint32_t generator_polynomial, uint32_t XOR_out,
             StCrcSettings* const config)
    : crc(crc),
      initial_crc(initial_crc),
      generator_polynomial(generator_polynomial),
      XOR_out(XOR_out),
      config(config)
{
}

bool init()
{
    // Initialize the CRC peripheral with the specified settings

    return true;
}

bool StCrc::compute(std::span<const uint32_t> data, uint32_t& result)
{
    // Compute the CRC for the given data using the CRC peripheral
    return true;
}

bool StCrc::compute(std::span<const uint16_t> data, uint16_t& result)
{
    return true;
}

bool StCrc::compute(std::span<const uint8_t> data, uint8_t& result)
{
    return true;
}

bool StCrc::compare(std::span<const uint32_t> data, uint32_t expected)
{
    // Compute the CRC for the given data and compare it with the expected value
    return computed_crc == expected;
}

bool StCrc::compare(std::span<const uint16_t> data, uint16_t expected)
{
    // Compute the CRC for the given data and compare it with the expected value
    return computed_crc == expected;
}

bool StCrc::compare(std::span<const uint8_t> data, uint8_t expected)
{
    // Compute the CRC for the given data and compare it with the expected value
    return computed_crc == expected;
}

}  // namespace EoT::StmH7