#pragma once
#include "board_traits.h"
#include "st_eth.h"
#include "st_gpio.h"
#include "st_rcc.h"
#include "st_sysclk.h"
#include "st_usart.h"

namespace EoT
{

bool board_init();

template <typename THw>
Board<THw>& get_board();

namespace StmH7
{

struct HwTraits
{
    using Ethernet = StEthernet;
    using Gpio = StGpio;
    using Rcc = StRcc;
    using Sysclk = StSysclk;
    using Usart = StUsart;
};

}  // namespace StmH7

using HwBoard = Board<StmH7::HwTraits>;

HwBoard& get_hw();

}  // namespace EoT
