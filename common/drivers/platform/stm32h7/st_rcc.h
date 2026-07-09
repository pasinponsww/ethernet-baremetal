#pragma once

#include <cstdint>
#include "stm32h723xx.h"

namespace EoT::StmH7
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
    H
};

enum class UartId : uint8_t
{
    U1 = 0,
    U2,
    U3,
    U4,
    U5,
    U6,
    U7,
    U8,
};

enum class DmaId : uint8_t
{
    D1 = 0,
    D2,
};

enum class EthernetClock : uint8_t
{
    Mac = 0,
    Tx,
    Rx
};

enum class AhbPeriph : uint8_t
{
    Crc
};

}  // namespace EoT::StmH7

#include "rcc.h"

namespace EoT::StmH7
{

class StRcc : public Rcc<StRcc>
{
public:
    explicit StRcc();

    /**
     * @brief Enable the clock for the specified Ethernet peripheral.
     * @param clock The Ethernet clock to enable.
     * @return true if the clock was enabled, false otherwise.
     */
    bool enable_eth_clock(EthernetClock clock);

    /**
     * @brief Enable the clock for the specified GPIO port.
     * @param port The GPIO port for which to enable the clock.
     * @return true if the clock was enabled, false otherwise.
     */
    bool enable_gpio_clock(GpioPort port);

    /**
     * @brief Enable the clock for the specified USART peripheral.
     * @param usart The USART peripheral for which to enable the clock.
     * @return true if the clock was enabled, false otherwise.
     */
    bool enable_uart_clock(UartId usart);

    /**
     * @brief Enable the clock for the specified DMA controller.
     * @param dma The DMA controller for which to enable the clock.
     * @return true if the clock was enabled, false otherwise.
     */
    bool enable_dma_clock(DmaId dma);

    /**
     * @brief Enable the clock for the specified AHB peripheral.
     * @param peripheral The AHB peripheral for which to enable the clock.
     * @return true if the clock was enabled, false otherwise.
     */
    bool enable_crc_clock(AhbPeriph peripheral);

    /**
     * @brief Enable the clock for the MDIOS peripheral.
     * @return true if the clock was enabled, false otherwise.
     */
    bool enable_mdio_clock();

    uint32_t get_freq() const;

private:
    RCC_TypeDef* const base_addr;
};

}  // namespace EoT::StmH7