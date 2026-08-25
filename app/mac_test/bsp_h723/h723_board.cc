#include "board.h"

#include <cstdint>
#include "st_eth_mac.h"
#include "st_gpio.h"
#include "st_rcc.h"
#include "st_sysclk.h"
#include "st_usart.h"

namespace EoT::StmH7
{

static constexpr uint32_t kUsartClockHz{64'000'000};

// RCC
StRcc rcc{};

// USART3 GPIO settings
StGpioSettings uart_io_settings{
    MODER::ALTERNATE_FUNCTION_MODE,
    OTYPE::PUSH_PULL,
    OSPEED::LOW_SPEED,
    PUPDR::NO_PU_PD,
    AF::AF7,
};

// USART3
StUsartSettings usart_params{
    USART3, 115200, kUsartClockHz, true, OversamplingMode::OS_16,
};

StUsart usart{&usart_params};

// USART3 pins: PD8 TX, PD9 RX
StGpio tx{GPIOD, 8, &uart_io_settings};
StGpio rx{GPIOD, 9, &uart_io_settings};

// Enables the SysTick tick that Utils::delay_ms() polls -- without this,
// delay_ms() spins forever (see delay.cc: g_ms_ticks is only ever
// incremented from the SysTick ISR that HAL_Init() arms).
StSysclk clock{Configuration::DEFAULT_HSI_64MHz};

// MAC settings. Locally-administered test address (02:00:00:00:00:01).
StEthMacSettings mac_settings{
    PhySpeed::Speed100M,
    PhyDuplex::Full,
    false,
    {0x02, 0x00, 0x00, 0x00, 0x00, 0x01},
    ReceiveFilterConfig{},
    FlowControlConfig{},
    1518U,
};

StEthMacParams mac_params{
    mac_settings,
    ETH,
};

StEthMac mac{mac_params};

// The one and only board instance
Board<StEthMac, StGpio, StRcc, StSysclk, StUsart> board{
    mac, tx, rx, rcc, clock, usart,
};

}  // namespace EoT::StmH7

namespace EoT
{

bool board_init()
{
    bool result = true;

    // GPIO clocks
    result &= StmH7::rcc.enable_gpio_clock(StmH7::GpioPort::D);  // USART3 TX/RX

    // Peripheral clocks
    result &= StmH7::rcc.enable_uart_clock(StmH7::UartId::U3);
    result &= StmH7::rcc.enable_eth_clock(StmH7::EthernetClock::Mac);
    result &= StmH7::rcc.enable_eth_clock(StmH7::EthernetClock::Tx);
    result &= StmH7::rcc.enable_eth_clock(StmH7::EthernetClock::Rx);

    // GPIO init
    result &= StmH7::tx.init();
    result &= StmH7::rx.init();

    // Peripheral init
    result &= StmH7::clock.init();
    result &= StmH7::usart.init();

    return result;
}

template <>
Board<StmH7::StEthMac, StmH7::StGpio, StmH7::StRcc, StmH7::StSysclk,
      StmH7::StUsart>&
get_board<StmH7::StEthMac, StmH7::StGpio, StmH7::StRcc, StmH7::StSysclk,
          StmH7::StUsart>()
{
    return StmH7::board;
}

HwBoard& get_hw()
{
    return get_board<StmH7::StEthMac, StmH7::StGpio, StmH7::StRcc,
                     StmH7::StSysclk, StmH7::StUsart>();
}

}  // namespace EoT
