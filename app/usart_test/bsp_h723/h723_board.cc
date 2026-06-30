#include "board.h"
#include "delay.h"
#include "st_gpio.h"
#include "st_sysclk.h"
#include "st_usart.h"
#include "stm32h7xx_hal.h"

inline uint8_t rxb;

namespace EoT::StmH7
{

// Configure RX and TX pins
StGpioSettings rx_settings{MODER::ALTERNATE_FUNCTION_MODE, OTYPE::PUSH_PULL,
                           OSPEED::LOW_SPEED, PUPDR::NO_PU_PD, AF::AF7};
StGpioSettings tx_settings{MODER::ALTERNATE_FUNCTION_MODE, OTYPE::PUSH_PULL,
                           OSPEED::LOW_SPEED, PUPDR::NO_PU_PD, AF::AF7};

// Configure UART
StUsartSettings usart_params{USART3, 9600, 8000000, false,
                             OversamplingMode::OS_16};

// Configure SysClk
StSysclk clock{Configuration::HSE_8MHZ_PLL};

StGpio rx{GPIOD, 9, &rx_settings};
StGpio tx{GPIOD, 8, &tx_settings};
StUsart usart{&usart_params};

}  // namespace EoT::StmH7

namespace EoT
{

Board<StmH7::StGpio, StmH7::StSysclk, StmH7::StUsart> board{
    .rx = StmH7::rx,
    .tx = StmH7::tx,
    .usart = StmH7::usart,
    .clock = StmH7::clock};

bool board_init()
{
    // Enable GPIOD clock
    RCC->AHB4ENR |= RCC_AHB4ENR_GPIODEN;

    // Enable USART3 clock
    RCC->APB1LENR |= RCC_APB1LENR_USART3EN;

    bool result = false;

    result = StmH7::clock.init();
    result = StmH7::rx.init();
    result = StmH7::tx.init();
    result = StmH7::usart.init();

    // Nested Vectored Interrupt Controller (NVIC) - a hardware block that sits between peripherals and CPU.
    // Manages every interrupt in the system.
    NVIC_SetPriority(USART3_IRQn, 0);  // priority (0 being highest)

    // Tells NVIC to start listening for USART3 interrupt requests.
    NVIC_EnableIRQ(USART3_IRQn);

    return result;
}

template <>
Board<StmH7::StGpio, StmH7::StSysclk, StmH7::StUsart>&
get_board<StmH7::StGpio, StmH7::StSysclk, StmH7::StUsart>(void)
{
    return board;
}

extern "C" void USART3_IRQHandler(void)
{
    if (StmH7::usart_params.base_addr->ISR & USART_ISR_RXNE_RXFNE)
    {
        if (board.usart.receive(rxb))
        {
            // Echo byte back
            board.usart.send(std::span<uint8_t>(&rxb, 1));
        }
    }
}

}  //namespace EoT
