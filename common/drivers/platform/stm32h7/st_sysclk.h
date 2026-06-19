/**
* @file st_sysclk.h
* @brief System clock configuration for STM32H7
* @author Bex Sawetrattanathumrong
* @date 6/19/2026
*/

#pragma once
#include "stm32h723xx.h"
#include "stm32h7xx_hal.h"
#include "sysclk.h"

namespace EoT
{
namespace StmH7
{

/**
* @brief Clock configuration options for STM32H7
* @note DEFAULT_HSI_64MHz is the default configuration
*       HSI_32MHZ is a lower frequency option for power saving
*       HSE_8MHZ_PLL uses an external crystal offer by stm32h723
*       HSI_16MHZ_DEPRECATED is a deprecated option that should not be used
*/

/* For Ethernet we should use HSE_8MHZ_PLL because it provides better accuracy */
/* Though it's slow and power consuming, byte-per-second was about 100KB/s */

enum class Configuration : uint8_t
{
    DEFAULT_HSI_64MHz = 0,
    HSI_32MHZ,
    HSE_8MHZ_PLL,
    HSI_16MHZ_DEPRECATED,
};

class StSysclk : EoT::Sysclk<StSysclk>
{
public:
    /**
    * @brief get_freq implementation for STM32H7
    * @return The frequency in Hz
    */
    uint32_t get_freq() const
    {
    }

    /**
    * @brief set_freq implementation for STM32H7
    * @param freq The frequency in Hz
    */
    void set_freq(uint32_t freq)
    {
    }

private:
    uint32_t hz;
    Configuration config;
};

}  // namespace StmH7
}  // namespace EoT
