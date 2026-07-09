#include "board.h"
#include "delay.h"
#include "st_gpio.h"
#include "st_mdio.h"
#include "st_sysclk.h"
#include "st_usart.h"
#include "stm32h7xx_hal.h"

uint8_t rxb;

namespace EoT::StmH7
{
// USART Pin Settings (AF7)
StGpioSettings rx_settings{MODER::ALTERNATE_FUNCTION_MODE, OTYPE::PUSH_PULL,
                           OSPEED::LOW_SPEED, PUPDR::NO_PU_PD, AF::AF7};
StGpioSettings tx_settings{MODER::ALTERNATE_FUNCTION_MODE, OTYPE::PUSH_PULL,
                           OSPEED::LOW_SPEED, PUPDR::NO_PU_PD, AF::AF7};
StUsartSettings usart_params{USART3, 115200, 64'000'000, true,
                             OversamplingMode::OS_16};

// System Clock (64MHz HSI)
StSysclk clock{Configuration::DEFAULT_HSI_64MHz};

StGpioSettings eth_mgmt_settings{MODER::ALTERNATE_FUNCTION_MODE,
                                 OTYPE::PUSH_PULL, OSPEED::VERY_HIGH_SPEED,
                                 PUPDR::NO_PU_PD, AF::AF12};

// Target the specific hardware register block for Ethernet MAC (ETH), PHY Address 0
StMdioParams mdio_params{MDIOS, 0U, 10'000U};

// Pin Instantiations
StGpio rx{GPIOD, 9, &rx_settings};
StGpio tx{GPIOD, 8, &tx_settings};
StGpio eth_mdio{GPIOA, 2, &eth_mgmt_settings};  // PA2 mapped to MDIO
StGpio eth_mdc{GPIOC, 1, &eth_mgmt_settings};   // PC1 mapped to MDC

StUsart usart{&usart_params};
StMdio mdio{mdio_params};
}  // namespace EoT::StmH7

namespace EoT
{
// Hook up the new pins into your structural Board wrapper
Board<StmH7::StGpio, StmH7::StSysclk, StmH7::StMdio, StmH7::StUsart> board{
    .rx = StmH7::rx,
    .tx = StmH7::tx,
    .eth_mdio = StmH7::eth_mdio,
    .eth_mdc = StmH7::eth_mdc,
    .mdio = StmH7::mdio,
    .usart = StmH7::usart,
    .clock = StmH7::clock};

bool board_init()
{
    bool result = false;

    // 1. Enable ALL necessary GPIO Ports (A and C for Ethernet MDIO/MDC, D for USART)
    RCC->AHB4ENR |=
        (RCC_AHB4ENR_GPIOAEN | RCC_AHB4ENR_GPIOCEN | RCC_AHB4ENR_GPIODEN);

    // 2. Enable Bus Clock
    RCC->APB1LENR |= RCC_APB1LENR_USART3EN;
    RCC->AHB1ENR |= RCC_AHB1ENR_ETH1MACEN;
    RCC->APB1HENR |= RCC_APB1HENR_MDIOSEN;

    // 4. Initialize Core Clock
    result = StmH7::clock.init();

    // 5. Initialize Physical MDIO/MDC Pins
    result = StmH7::eth_mdio.init();
    result = StmH7::eth_mdc.init();

    // 6. Initialize USART Pins and Peripheral
    result = StmH7::rx.init();
    result = StmH7::tx.init();
    result = StmH7::usart.init();

    // 7. Initialize the MDIO Register Driver logic
    result = (StmH7::mdio.init() == EoT::MdioStatus::OK);

    // 8. Configure Interrupts
    NVIC_SetPriority(USART3_IRQn, 0);
    NVIC_EnableIRQ(USART3_IRQn);

    return result;
}

template <>
Board<StmH7::StGpio, StmH7::StSysclk, StmH7::StMdio, StmH7::StUsart>&
get_board<StmH7::StGpio, StmH7::StSysclk, StmH7::StMdio, StmH7::StUsart>(void)
{
    return board;
}

HwBoard& get_hw()
{
    return get_board<StmH7::StGpio, StmH7::StSysclk, StmH7::StMdio,
                     StmH7::StUsart>();
}

extern "C" void USART3_IRQHandler(void)
{
    if (StmH7::usart.get_addr()->ISR & USART_ISR_RXNE_RXFNE)
    {
        if (board.usart.receive(rxb))
        {
            std::span<const uint8_t> txbuf(&rxb, 1);
            board.usart.send(txbuf);
        }
    }
}
}  // namespace EoT
