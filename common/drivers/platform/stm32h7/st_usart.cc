#include "st_usart.h"

namespace EoT::StmH7
{

bool StUsart::init()
{

    if (base_addr == nullptr)
        return false;

    // Make sure UART is disabled before config
    base_addr->CR1 &= ~USART_CR1_UE;

    // 8 data bits, no parity, 1 stop bit
    base_addr->CR1 &= ~USART_CR1_M;
    base_addr->CR1 &= ~USART_CR1_PCE;
    base_addr->CR2 &= ~USART_CR2_STOP;

    // TODO: Configure baud rate
    //base_addr->BRR =

    // Enable USART, RE, and TE
    base_addr->CR1 |= USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;

    return true;
}

bool StUsart::send(const std::span<const uint8_t> data)
{
    // Send each byte of data
    for (const auto byte : data)
    {
        // Wait until TDR is empty
        while (!(base_addr->ISR & USART_ISR_TXE_TXFNF))
        {
        }

        base_addr->TDR = byte;
    }

    // Wait for transmission to complete
    while (!(base_addr->ISR & USART_ISR_TC))
    {
    }

    return true;
}

bool StUsart::receive(uint8_t& byte)
{
    // If data isn't ready
    if (!(base_addr->ISR & USART_ISR_RXNE_RXFNE))
    {
        return false;
    }

    byte = base_addr->RDR;

    return true;
}

}  // namespace EoT::StmH7
