#include "st_crc.h"

namespace EoT::StmH7
{

/** @brief This is a constant for the bit width of the CRC control register's reverse output bit */
<<<<<<< HEAD
<<<<<<< HEAD
static constexpr uint8_t CRC_CR_REV_OUT_BitWidth{1};
static constexpr uint8_t CRC_CR_REV_IN_BitWidth{2};
static constexpr uint8_t CRC_CR_POLYSIZE_BitWidth{2};

/** @brief Loads a value into the CRC data register */

// 8/16/32 bit data register variant
inline void StCrc::feed(uint8_t value)
=======
static constexpr uint8_t kCRC_CR_REV_OUT_BitWidth{1};
static constexpr uint8_t kCRC_CR_REV_IN_BitWidth{2};
static constexpr uint8_t kCRC_CR_POLYSIZE_BitWidth{2};

/** @brief Loads a value into the CRC data register */

inline void StCrc::load(uint8_t value)
>>>>>>> b124d2e (structure + skeleton for crc feature)
=======
static constexpr uint8_t CRC_CR_REV_OUT_BitWidth{1};
static constexpr uint8_t CRC_CR_REV_IN_BitWidth{2};
static constexpr uint8_t CRC_CR_POLYSIZE_BitWidth{2};

/** @brief Loads a value into the CRC data register */

// 8/16/32 bit data register variant 
inline void StCrc::feed(uint8_t value)
>>>>>>> f69f824 (implementation for crc)
{
    volatile uint8_t* dr8 = reinterpret_cast<volatile uint8_t*>(&crc->DR);
    *dr8 = value;
}
<<<<<<< HEAD
<<<<<<< HEAD
inline void StCrc::feed(uint16_t value)
=======
inline void StCrc::load(uint16_t value)
>>>>>>> b124d2e (structure + skeleton for crc feature)
=======
inline void StCrc::feed(uint16_t value)
>>>>>>> f69f824 (implementation for crc)
{
    volatile uint16_t* dr16 = reinterpret_cast<volatile uint16_t*>(&crc->DR);
    *dr16 = value;
}
<<<<<<< HEAD
<<<<<<< HEAD
inline void StCrc::feed(uint32_t value)
=======
inline void StCrc::load(uint32_t value)
>>>>>>> b124d2e (structure + skeleton for crc feature)
=======
inline void StCrc::feed(uint32_t value)
>>>>>>> f69f824 (implementation for crc)
{
    volatile uint32_t* dr32 = reinterpret_cast<volatile uint32_t*>(&crc->DR);
    *dr32 = value;
}

<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> f69f824 (implementation for crc)
StCrc::StCrc(const StCrcParams& params)
    : crc(params.crc),
      settings(params.settings),
      initial_crc(params.initial_crc),
      generator_polynomial(params.generator_polynomial),
      xor_out(params.XOR_out)
<<<<<<< HEAD
{
}

bool StCrc::init()
{
    // If crc points to nullptr, return false
    if (crc == nullptr)
    {
        return false;
    }

    // Init the CRC
    crc->INIT = initial_crc;

    // Set Register Reverse OUT/IN/POLY_SIZE
    // Reverse Output
    SetReg(&(crc->CR), static_cast<uint32_t>(settings.reverse_out),
           CRC_CR_REV_OUT_Pos, CRC_CR_REV_OUT_BitWidth);

    // Reverse Input
    SetReg(&(crc->CR), static_cast<uint32_t>(settings.reverse_in),
           CRC_CR_REV_IN_Pos, CRC_CR_REV_IN_BitWidth);

    // Polynomial Size
    SetReg(&(crc->CR), static_cast<uint32_t>(settings.poly_size),
           CRC_CR_POLYSIZE_Pos, CRC_CR_POLYSIZE_BitWidth);

    // Generate the polynomial CRC
    crc->POL = generator_polynomial;

    // Reset the CRC
    crc->CR |= CRC_CR_RESET;
=======
StCrc::StCrc(CRC_TypeDef* const crc, uint32_t initial_crc,
             uint32_t generator_polynomial, uint32_t XOR_out,
             StCrcSettings* const config)
    : crc(crc),
      initial_crc(initial_crc),
      generator_polynomial(generator_polynomial),
      XOR_out(XOR_out),
      config(config)
=======
>>>>>>> f69f824 (implementation for crc)
{
}

bool StCrc::init()
{
<<<<<<< HEAD
    // Initialize the CRC peripheral with the specified settings
>>>>>>> b124d2e (structure + skeleton for crc feature)
=======
    // If crc points to nullptr, return false
    if(crc == nullptr)
    {
        return false;
    }
>>>>>>> f69f824 (implementation for crc)

    // Init the CRC
    crc->INIT = initial_crc;
    
    // Set Register Reverse OUT/IN/POLY_SIZE
    // Reverse Output
    SetReg(&(crc->CR), static_cast<uint32_t>(settings.reverse_out), 
        CRC_CR_REV_OUT_Pos, CRC_CR_REV_OUT_BitWidth);
    
    // Reverse Input
    SetReg(&(crc->CR), static_cast<uint32_t>(settings.reverse_in),
        CRC_CR_REV_IN_Pos, CRC_CR_REV_IN_BitWidth);

    // Polynomial Size
    SetReg(&(crc->CR), static_cast<uint32_t>(settings.poly_size),
        CRC_CR_POLYSIZE_Pos, CRC_CR_POLYSIZE_BitWidth);

    // Generate the polynomial CRC
    crc->POL = generator_polynomial;

    // Reset the CRC
    crc->CR |= CRC_CR_RESET; 
    
    return true;
}

bool StCrc::compute(std::span<const uint32_t> data, uint32_t& result)
{
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> f69f824 (implementation for crc)
    // CRC Reset (The computation need to basically refresh it's bit to compute the next set of byte given)
    crc->CR |= CRC_CR_RESET;

    // Compute the bits given the input
<<<<<<< HEAD
    for (auto word : data)
=======
    for(auto word : data)
>>>>>>> f69f824 (implementation for crc)
    {
        feed(word);
    }

    // The result of new CRC calculation byte feed into 'result'
<<<<<<< HEAD
    result = crc->DR ^ xor_out;

    return true;
}

bool StCrc::compute(std::span<const uint16_t> data, uint32_t& result)
{
    // CRC Reset (The computation need to basically refresh it's bit to compute the next set of byte given)
    crc->CR |= CRC_CR_RESET;

    // Compute the bits given the input
    for (auto half_word : data)
    {
        feed(half_word);
    }

    // The result of new CRC calculation byte feed into 'result'
    result = crc->DR ^ xor_out;

    return true;
}

bool StCrc::compute(std::span<const uint8_t> data, uint32_t& result)
{
    // CRC Reset (The computation need to basically refresh it's bit to compute the next set of byte given)
    crc->CR |= CRC_CR_RESET;

    // Compute the bits given the input
    for (auto byte : data)
    {
        feed(byte);
    }

    // The result of new CRC calculation byte feed into 'result'
    result = crc->DR ^ xor_out;

=======
    // Compute the CRC for the given data using the CRC peripheral
=======
    result = crc->DR ^ XOR_out;

>>>>>>> f69f824 (implementation for crc)
    return true;
}

bool StCrc::compute(std::span<const uint16_t> data, uint16_t& result)
{
    // CRC Reset (The computation need to basically refresh it's bit to compute the next set of byte given)
    crc->CR |= CRC_CR_RESET;

    // Compute the bits given the input
    for(auto half_word : data)
    {
        feed(half_word);
    }

    // The result of new CRC calculation byte feed into 'result'
    result = crc->DR ^ XOR_out;

    return true;
}

bool StCrc::compute(std::span<const uint8_t> data, uint8_t& result)
{
<<<<<<< HEAD
>>>>>>> b124d2e (structure + skeleton for crc feature)
=======
    // CRC Reset (The computation need to basically refresh it's bit to compute the next set of byte given)
    crc->CR |= CRC_CR_RESET;

    // Compute the bits given the input
    for(auto byte : data)
    {
        feed(byte);
    }

    // The result of new CRC calculation byte feed into 'result'
    result = crc->DR ^ XOR_out;

>>>>>>> f69f824 (implementation for crc)
    return true;
}

bool StCrc::compare(std::span<const uint32_t> data, uint32_t expected)
{
<<<<<<< HEAD
<<<<<<< HEAD
    uint32_t expected_crc;  // we normalized our result to CRC-32

    if (compute(data, expected_crc))
=======
    uint32_t expected_crc; // we normalized our result to CRC-32 

    if(compute(data, expected_crc))
>>>>>>> f69f824 (implementation for crc)
    {
        return (expected == expected_crc);
    }

    return false;
<<<<<<< HEAD
}

bool StCrc::compare(std::span<const uint16_t> data, uint32_t expected)
{
    uint32_t expected_crc;  // we normalized our result to CRC-32

    if (compute(data, expected_crc))
    {
        return (expected == expected_crc);
    }

    return false;
}

bool StCrc::compare(std::span<const uint8_t> data, uint32_t expected)
{
    uint32_t expected_crc;  // we normalized our result to CRC-32

    if (compute(data, expected_crc))
    {
        return (expected == expected_crc);
    }

    return false;
=======
    // Compute the CRC for the given data and compare it with the expected value
    return computed_crc == expected;
=======
>>>>>>> f69f824 (implementation for crc)
}

bool StCrc::compare(std::span<const uint16_t> data, uint16_t expected)
{
    uint32_t expected_crc; // we normalized our result to CRC-32 

    if(compute(data, expected_crc))
    {
        return (expected == expected_crc);
    }

    return false;
}

bool StCrc::compare(std::span<const uint8_t> data, uint8_t expected)
{
<<<<<<< HEAD
    // Compute the CRC for the given data and compare it with the expected value
    return computed_crc == expected;
>>>>>>> b124d2e (structure + skeleton for crc feature)
=======
    uint32_t expected_crc; // we normalized our result to CRC-32 

    if(compute(data, expected_crc))
    {
        return (expected == expected_crc);
    }
    
    return false;
>>>>>>> f69f824 (implementation for crc)
}

}  // namespace EoT::StmH7