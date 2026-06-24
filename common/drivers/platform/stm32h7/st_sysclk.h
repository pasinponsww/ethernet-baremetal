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
* @note DEFAULT_HSI_64MHz is the default configuration (HCLK 64 MHz).
*       HSI_32MHZ is a lower frequency option for power saving (HCLK 32 MHz).
*       HSE_8MHZ_PLL runs straight off the 8 MHz HSE bypass (HCLK 8 MHz).
*         WARNING: 8 MHz HCLK is below the Ethernet MAC minimum (25 MHz) and
*         must NOT be used for networking - throughput collapses (~100 KB/s).
*       ETH_RMII_200MHZ is the Ethernet-grade option: HSE 8 MHz -> PLL,
*         200 MHz SYSCLK / 100 MHz HCLK. The 50 MHz RMII_REF_CLK is supplied
*         externally by the LAN8742 on PA1, so it is not generated here.
*/

enum class Configuration : uint8_t
{
    DEFAULT_HSI_64MHz = 0,
    HSI_32MHZ,
    HSE_8MHZ_PLL,
    ETH_RMII_200MHZ,  // HSE 8 MHz -> PLL, 200 MHz sys / 100 MHz HCLK
};

class StSysclk : public EoT::Sysclk<StSysclk>
{
public:
    /**
    * @brief Constructor for StSysclk
    * @param[in] config The configuration to use for the system clock
    */
    explicit StSysclk(Configuration config);

    /**
    * @brief Initialize the system clock
    * @return true if the initialization was successful, false otherwise
    */
    bool init();

    /**
    * @brief get_freq implementation for STM32H7
    * @return The frequency in Hz
    */
    uint32_t get_freq() const
    {
        return hz;
    }

    /**
    * @brief Whether the current clock tree can drive the Ethernet MAC/DMA.
    * @note  The STM32H7 ETH peripheral requires HCLK >= 25 MHz. The 50 MHz
    *        RMII_REF_CLK is supplied externally by the LAN8742 PHY (PA1), so
    *        it is NOT validated here - this only checks the AHB/HCLK floor.
    * @return true if HCLK >= 25 MHz.
    */
    bool is_eth_capable() const;

private:
    uint32_t hz;
    Configuration config;
};

}  // namespace StmH7
}  // namespace EoT
