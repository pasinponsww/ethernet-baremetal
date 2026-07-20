#include "board.h"

#include <cstdint>
#include <span>
#include "lan8742.h"
#include "st_eth_mdio.h"
#include "st_gpio.h"
#include "st_rcc.h"
#include "st_usart.h"

namespace EoT::StmH7
{

static constexpr uint32_t kUsartClockHz{64'000'000};
static constexpr uint32_t kEthCsrClockHz{100'000'000};
static constexpr uint32_t kMdioTimeoutUs{1000U};

// LAN8742 PHY address on the board. Default strap on the Nucleo/H723 boards is
// 0; the test also scans 0..31 as a fallback if this does not respond.
static constexpr uint8_t kPhyAddr{0U};

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

// Ethernet MDIO/MDC GPIO settings
StGpioSettings eth_mdio_io_settings{
    MODER::ALTERNATE_FUNCTION_MODE,
    OTYPE::PUSH_PULL,
    OSPEED::VERY_HIGH_SPEED,
    PUPDR::NO_PU_PD,
    AF::AF11,
};

// USART3
StUsartSettings usart_params{
    USART3, 115200, kUsartClockHz, true, OversamplingMode::OS_16,
};

StUsart usart{&usart_params};

// USART3 pins: PD8 TX, PD9 RX
StGpio tx{GPIOD, 8, &uart_io_settings};
StGpio rx{GPIOD, 9, &uart_io_settings};

// Ethernet MDIO pins: PA2 MDIO, PC1 MDC
StGpio eth_mdio_pin{GPIOA, 2, &eth_mdio_io_settings};
StGpio eth_mdc_pin{GPIOC, 1, &eth_mdio_io_settings};

// Ethernet MDIO driver
StEthMdioSettings eth_mdio_settings{
    MdioClockRange::Div42,
    MdioClause::Clause22,
};

StEthMdioConfig eth_mdio_config{
    ETH,
    kEthCsrClockHz,
    kMdioTimeoutUs,
    eth_mdio_settings,
};

StEthMdio eth_mdio{eth_mdio_config};

// LAN8742 PHY driver on top of the MDIO backend.
PhySettings phy_settings{
    PhySpeed::Speed100M,
    PhyDuplex::Full,
};

PhyParams<StEthMdio> phy_params{
    eth_mdio,
    kPhyAddr,
    phy_settings,
};

Lan8742<StEthMdio> phy{phy_params};

// The one and only board instance
Board<StEthMdio, StGpio, StRcc, StUsart> board{
    eth_mdio, tx, rx, rcc, usart, phy,
};

}  // namespace EoT::StmH7

namespace EoT
{

bool board_init()
{
    bool result = true;

    // GPIO clocks
    result &= StmH7::rcc.enable_gpio_clock(StmH7::GpioPort::A);  // ETH_MDIO
    result &= StmH7::rcc.enable_gpio_clock(StmH7::GpioPort::C);  // ETH_MDC
    result &= StmH7::rcc.enable_gpio_clock(StmH7::GpioPort::D);  // USART3 TX/RX

    // Peripheral clocks
    result &= StmH7::rcc.enable_uart_clock(StmH7::UartId::U3);
    result &= StmH7::rcc.enable_eth_clock(StmH7::EthernetClock::Mac);
    result &= StmH7::rcc.enable_eth_clock(StmH7::EthernetClock::Tx);
    result &= StmH7::rcc.enable_eth_clock(StmH7::EthernetClock::Rx);

    // GPIO init
    result &= StmH7::tx.init();
    result &= StmH7::rx.init();
    result &= StmH7::eth_mdio_pin.init();
    result &= StmH7::eth_mdc_pin.init();

    // Peripheral init
    result &= StmH7::usart.init();
    result &= StmH7::eth_mdio.init() == EthMdioStatus::Ok;

    return result;
}

template <>
Board<StmH7::StEthMdio, StmH7::StGpio, StmH7::StRcc, StmH7::StUsart>&
get_board<StmH7::StEthMdio, StmH7::StGpio, StmH7::StRcc, StmH7::StUsart>()
{
    return StmH7::board;
}

HwBoard& get_hw()
{
    return get_board<StmH7::StEthMdio, StmH7::StGpio, StmH7::StRcc,
                     StmH7::StUsart>();
}

}  // namespace EoT
