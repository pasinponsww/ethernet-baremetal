/**
* @file st_crc.h
* @author Bex Sawetrattanathumrong
*/

#pragma once
#include "crc.h"
#include "reg_helpers.h"
#include "stm32h723xx.h"

namespace EoT::StmH7
{

/**
* @brief Used to control the reversal of the bit order of the output data
*/
enum class CrcRevOut : uint8_t
{
    NO_REV = 0,
    REVERSED
};

/**
 * @brief This is to control the reversal of the bit order of each input data
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
    BIT_SIZE_32 = 0,
    BIT_SIZE_16,
    BIT_SIZE_8,
    BIT_SIZE_7
};

struct StCrcSettings
{
    CrcRevOut reverse_out;
    CrcRevIn reverse_in;
    CrcPolySize poly_size;
};

class StCrc : public EoT::Crc<StCrc>
{
public:
    StCrc(CRC_TypeDef* const crc, uint32_t initial_crc,
          uint32_t generator_polynomial = 0x4C11DB7, uint32_t XOR_out,
          StCrcSettings* const config);

    /**
     * @brief Initialize the CRC peripheral
     * @return true if the initialization was successful, false otherwise
     */
    bool init();

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
    void load(uint8_t value);
    void load(uint16_t value);
    void load(uint32_t value);

    CRC_TypeDef* const crc{CRC};  // pointer to the CRC peripheral
    uint32_t initial_crc;
    uint32_t generator_polynomial{0x4C11DB7};  // default 0x4C11DB7
    uint32_t XOR_out;
};
}  // namespace EoT::StmH7