#pragma once

#include "gpio.h"
#include "st_eth.h"
#include "st_gpio.h"
#include "st_rcc.h"
#include "st_sysclk.h"
#include "st_usart.h"
#include "usart.h"

namespace EoT
{

template <typename TEthernet, typename TGpio, typename TRcc, typename TSysclk,
          typename TUsart>
struct Board
{
    TEthernet& eth;
    Gpio<TGpio>& tx;
    Gpio<TGpio>& rx;
    TRcc& rcc;
    TSysclk& clock;
    Usart<TUsart>& usart;
};

bool board_init();

template <typename TEthernet, typename TGpio, typename TRcc, typename TSysclk,
          typename TUsart>
Board<TEthernet, TGpio, TRcc, TSysclk, TUsart>& get_board();

using HwBoard = Board<StmH7::StEthernet, StmH7::StGpio, StmH7::StRcc,
                      StmH7::StSysclk, StmH7::StUsart>;

HwBoard& get_hw();

}  // namespace EoT
