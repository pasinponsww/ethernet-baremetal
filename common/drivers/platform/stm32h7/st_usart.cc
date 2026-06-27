#include "st_usart.h"

namespace EoT::StmH7
{

StUsart::StUsart(StUsartSettings* config) : params(config)
{
}

bool StUsart::init()
{

    if (params->base_addr == nullptr)
    {
        return false;
    }

    // Make sure UART is disabled before config
    params->base_addr->CR1 &= ~USART_CR1_UE;

    // Clear any active error flags left over from previous runs
    params->base_addr->ICR = USART_ICR_ORECF | USART_ICR_FECF | USART_ICR_NECF;

    // FIFO mode
    if (params->fifo_mode)
    {
        // Set the FIFOEN bit to enable FIFO mode
        params->base_addr->CR1 |= USART_CR1_FIFOEN;
    }
    else
    {
        // Clear the FIFOEN bit to disable FIFO mode
        params->base_addr->CR1 &= ~USART_CR1_FIFOEN;
    }

    // 8 data bits, no parity, 1 stop bit
    params->base_addr->CR1 &= ~USART_CR1_M;
    params->base_addr->CR1 &= ~USART_CR1_PCE;
    params->base_addr->CR2 &= ~USART_CR2_STOP;

    // Configure Sampling Rate & baud rate
    uint16_t usart_div{};

    if (params->sampling_mode == OversamplingMode::OS_16)
    {
        //BRR[16:0] = USARTDIV
        params->base_addr->CR1 &= ~USART_CR1_OVER8_Msk;
        usart_div = params->clk_freq / params->baud_rate;
        params->base_addr->BRR = usart_div;
    }

    else
    {
        // Oversampling OS_8

        uint16_t top = usart_div & 0xFFF0;
        uint16_t bottom = (usart_div & 0x000F) >> 1;
        params->base_addr->BRR = top | bottom;
        // BRR[15:4] = USARTDIV[15:4] (copied directly)
        // BRR[2:0]  = USARTDIV[3:0] >> 1 (shifted right, bit 0 discarded)
        // BRR[3]    = 0 (must be kept cleared, shift handles this naturally)
        params->base_addr->CR1 |= USART_CR1_OVER8_Msk;
        usart_div = (2 * params->clk_freq) / params->baud_rate;
        params->base_addr->BRR = usart_div;
    }

    // Enable USART, RE, and TE
    params->base_addr->CR1 |= USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;

    // Enable RXNE Interrupt
    params->base_addr->CR1 |= USART_CR1_RXNEIE_RXFNEIE;

    return true;
}

// uses polling
bool StUsart::send(const std::span<const uint8_t> data)
{
    // Send each byte
    for (const auto byte : data)
    {
        while (!(params->base_addr->ISR & USART_ISR_TXE_TXFNF))
        {
        }
        params->base_addr->TDR = byte;
    }

    // Wait for transmission to complete (TXFIFO and shift register are empty)
    while (!(params->base_addr->ISR & USART_ISR_TC))
    {
    }

    return true;
}

// uses interrupts
bool StUsart::receive(uint8_t& byte)
{
    //
    if (!(params->base_addr->ISR & USART_ISR_RXNE_RXFNE))
    {
        return false;
    }

    // Read RDR (or oldest data in RXFIFO)
    byte = params->base_addr->RDR;
    return true;
}

}  // namespace EoT::StmH7
