#include "st_rcc.h"

namespace EoT::StmH7
{

static inline bool enable_register_bit(volatile uint32_t& reg, uint32_t bit)
{
    const uint32_t mask = 1u << bit;

    if (reg & mask)
    {
        return false;  // already enabled
    }

    reg |= mask;

    const volatile uint32_t dummy = reg;
    (void)dummy;

    return true;
}

StRcc::StRcc() : base_addr(RCC)
{
}

bool StRcc::enable_eth_clock(EthernetClock clock)
{
    switch (clock)
    {
        case EthernetClock::Mac:
            return enable_register_bit(base_addr->AHB1ENR, 15);

        case EthernetClock::Tx:
            return enable_register_bit(base_addr->AHB1ENR, 16);

        case EthernetClock::Rx:
            return enable_register_bit(base_addr->AHB1ENR, 17);

        default:
            return false;
    }
}

bool StRcc::enable_gpio_clock(GpioPort port)
{
    if (port < GpioPort::A || port > GpioPort::H)
    {
        return false;
    }

    return enable_register_bit(base_addr->AHB4ENR, static_cast<uint32_t>(port));
}

bool StRcc::enable_uart_clock(UartId usart)
{
    switch (usart)
    {
        case UartId::U1:
            return enable_register_bit(base_addr->APB2ENR, 4);

        case UartId::U2:
            return enable_register_bit(base_addr->APB1LENR, 17);

        case UartId::U3:
            return enable_register_bit(base_addr->APB1LENR, 18);

        case UartId::U4:
            return enable_register_bit(base_addr->APB1LENR, 19);

        case UartId::U5:
            return enable_register_bit(base_addr->APB1LENR, 20);

        case UartId::U6:
            return enable_register_bit(base_addr->APB2ENR, 5);

        case UartId::U7:
            return enable_register_bit(base_addr->APB1LENR, 30);

        case UartId::U8:
            return enable_register_bit(base_addr->APB1LENR, 31);

        default:
            return false;
    }
}

bool StRcc::enable_dma_clock(DmaId dma)
{
    switch (dma)
    {
        case DmaId::D1:
            return enable_register_bit(base_addr->AHB1ENR, 0);

        case DmaId::D2:
            return enable_register_bit(base_addr->AHB1ENR, 1);

        default:
            return false;
    }
}

bool StRcc::enable_crc_clock(AhbPeriph peripheral)
{
    switch (peripheral)
    {
        case AhbPeriph::Crc:
            return enable_register_bit(base_addr->AHB4ENR, 19);

        default:
            return false;
    }
}

uint32_t StRcc::get_freq() const
{
    return freq;
}

}  // namespace EoT::StmH7