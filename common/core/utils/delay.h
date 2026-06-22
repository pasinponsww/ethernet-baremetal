/**
* @file delay.h
* @brief Delay utilities for STM32H7
* @author Bex Sawetrattanathumrong
* @date 6/19/2026
*/

#pragma once
#include <cstdint>

/* IncDelayTicks has C linkage so it can be called from the SysTick ISR */
#ifdef __cplusplus
extern "C"
{
#endif

    /**
 * @brief Increments the millisecond tick counter
 * Intended to be called from the SysTick interrupt handler (1 kHz tick).
 */
    void IncDelayTicks(void);

#ifdef __cplusplus
}
#endif

namespace EoT::Utils
{

/**
 * @brief Returns the current millisecond tick count since boot
 * @return Number of elapsed SysTick ticks (milliseconds)
 */
uint32_t get_ms_ticks(void);

/**
 * @brief Busy-waits for the specified number of milliseconds
 * @param ms Number of milliseconds to wait
 */
void delay_ms(uint32_t ms);

}  // namespace EoT::Utils
