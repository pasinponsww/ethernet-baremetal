/**
* @file st_rcc.h
* @brief STM32H7 RCC (Reset and Clock Control) driver implementation
* @author Bex Sawetrattanathumrong
* @date 7/1/2026
*/

#include "rcc.h"
#include "stm32h732xx.h"
#include <cstdint>

namespace EoT
{
namespace StmH7
{
enum class GpioPort : uint8_t
{
    A = 0,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
};

enum class Usart : uint8_t
{
    Usart1 = 0,
    Usart2,
    Usart3,
    Uart4,
    Uart5,
};

enum class EthernetClock : uint8_t
{
    Mac = 0,
    Tx,
    Rx,
};

enum class Dma : uint8_t
{
    Dma1 = 0,
    Dma2,
};

enum class AhbPeriph : uint8_t
{
    Crc,
};

class StRcc : public EoT::Rcc<StRcc>
{
public:
    StRcc() : base_addr(RCC) {}

    /**
    * @brief Enables the clock for the specified peripheral.
    * @param peripheral The peripheral for which to enable the clock.
    * @return true if the clock was successfully enabled, false otherwise.
    */
    bool enable_eth_clock(EthernetClock clock);

    /**
    * @brief Enables the clock for the specified GPIO port.
    * @param port The GPIO port for which to enable the clock.
    * @return true if the clock was successfully enabled, false otherwise.
    */
    bool enable_gpio_clock(GpioPort port);

    /**
    * @brief Enables the clock for the specified USART.
    * @param usart The USART for which to enable the clock.
    * @return true if the clock was successfully enabled, false otherwise.
    */
    bool enable_uart_clock(Usart usart);

    /**
    * @brief Enables the clock for the specified DMA controller.
    * @param dma The DMA controller for which to enable the clock.
    * @return true if the clock was successfully enabled, false otherwise.
    */
    bool enable_dma_clock(Dma dma);

    /**
    * @brief Enables the clock for the specified AHB peripheral.
    * @param peripheral The AHB peripheral for which to enable the clock.
    * @return true if the clock was successfully enabled, false otherwise.
    */
    bool enable_crc_clock(AhbPeriph peripheral);

    /**
    * @brief Gets the frequency of the RCC clock.
    * @return The frequency of the RCC clock in Hz.
    */
    uint32_t get_freq() const;

private:
    RCC_TypeDef* const base_addr;

    // get_freq() is here to provide the frequency of the RCC clock.
    uint32_t freq{0}; 

};
}  // namespace StmH7
}
