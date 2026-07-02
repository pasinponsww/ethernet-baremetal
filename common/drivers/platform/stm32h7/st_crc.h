/**
* @file st_crc.h
* @author Bex Sawetrattanathumrong
*/

#pragma once
#include "crc.h"
#include "reg_helpers.h"
#include "stm32h723xx.h"

/* The Ethernet CRC is basically the standard CRC-32 with this generator-polynomial-byte: 0x4C11DB7 */
/* @note Be aware that we normalized all of our result to CRC-32 byte when compare() but compute() could be scale from 8/16/32 */
namespace EoT::StmH7
{

/**
* @brief Used to control the reversal of the bit order of the output data
* This is for the endianess of the CRC result. If the output is reversed, the least significant bit will be output first.
*/
enum class CrcRevOut : uint8_t
{
    NO_REV = 0,
    REVERSED
};

/**
 * @brief This is to control the reversal of the bit order of each input data
 * This is the endianess of the input data. If the input is reversed, the least significant bit will be input first.
 */
enum class CrcRevIn : uint8_t
{
    NO_REV = 0,
    BYTE,
    HALF_WORD,
    WORD
};

/**
 * @brief This is to control the polynomial size of the CRC calculation
 * @note The polynomial size can be 32, 16, 8, or 7 bits. 
*/
enum class CrcPolySize : uint8_t
{
    SIZE_32 = 0,
    SIZE_16,
    SIZE_8,
    SIZE_7
};

// Simplified struct for CRC settings
struct StCrcSettings
{
    CrcRevOut reverse_out;
    CrcRevIn reverse_in;
    CrcPolySize poly_size;
};

/**
* @brief This struct is used to configure the CRC peripheral
*/
struct StCrcParams
{
    StCrcSettings settings;
    CRC_TypeDef* crc;
    uint32_t initial_crc;
    uint32_t generator_polynomial{0x4C11DB7};
    uint32_t XOR_out;
};

class StCrc : public EoT::Crc<StCrc>
{
public:
    explicit StCrc(const StCrcParams& params);

    /**
     * @brief Initialize the CRC peripheral
     * @return true if the initialization was successful, false otherwise
     */
    bool init();

    /* Assume the compute() function doesn't know at the runtime that it will be a 
    fix-size of byte (I populate the function into 3 variant of 32/16/8 bytes)*/

    /**
     * @brief Compute the CRC for the given data
     * @param data The data to compute the CRC for
     * @param result The computed CRC value
     * @return true if the computation was successful, false otherwise
     */
    bool compute(std::span<const uint32_t> data, uint32_t& result);
    bool compute(std::span<const uint16_t> data, uint32_t& result);
    bool compute(std::span<const uint8_t> data, uint32_t& result);

    /**
     * @brief Compare the computed CRC with the expected value
     * @param data The data to compute the CRC for
     * @param expected The expected CRC value
     * @return true if the computed CRC matches the expected value, false otherwise
     */
    bool compare(std::span<const uint32_t> data, uint32_t expected);
    bool compare(std::span<const uint16_t> data, uint32_t expected);
    bool compare(std::span<const uint8_t> data, uint32_t expected);

private:
    /**
    * @brief Helper function for loading data into the CRC peripheral
    * @param value The value to load into the CRC peripheral
    */
    void feed(uint8_t value);
    void feed(uint16_t value);
    void feed(uint32_t value);

    /* All of the CRC params */

    CRC_TypeDef* const crc;
    StCrcSettings settings;
    uint32_t initial_crc;
    uint32_t generator_polynomial;
    uint32_t xor_out;
};
}  // namespace EoT::StmH7