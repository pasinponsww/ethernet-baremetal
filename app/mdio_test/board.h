#pragma once

#include "gpio.h"
#include "mdio.h"
#include "st_gpio.h"
#include "st_mdio.h"
#include "st_sysclk.h"
#include "st_usart.h"
#include "sysclk.h"
#include "usart.h"

namespace EoT
{

template <typename TGpio, typename TClk, typename TMdio, typename TUsart>
struct Board
{
    Gpio<TGpio>& rx;
    Gpio<TGpio>& tx;
    Gpio<TGpio>& eth_mdio;
    Gpio<TGpio>& eth_mdc;
    Mdio<TMdio>& mdio;
    Usart<TUsart>& usart;
    Sysclk<TClk>& clock;
};

bool board_init(void);

template <typename TGpio, typename TClk, typename TMdio, typename TUsart>
Board<TGpio, TClk, TMdio, TUsart>& get_board(void);

using HwBoard =
    Board<StmH7::StGpio, StmH7::StSysclk, StmH7::StMdio, StmH7::StUsart>;

HwBoard& get_hw();

}  // namespace EoT