#pragma once

#include "eth_mac.h"
#include "eth_mtl.h"
#include "gpio.h"
#include "st_eth_mac.h"
#include "st_eth_mtl.h"
#include "st_gpio.h"
#include "st_rcc.h"
#include "st_sysclk.h"
#include "st_usart.h"
#include "usart.h"

namespace EoT
{

template <typename TMac, typename TMtl, typename TGpio, typename TRcc,
          typename TSysclk, typename TUsart>
struct Board
{
    Mac<TMac>& mac;
    Mtl<TMtl>& mtl;
    Gpio<TGpio>& tx;
    Gpio<TGpio>& rx;
    TRcc& rcc;
    TSysclk& clock;
    Usart<TUsart>& usart;
};

bool board_init();

template <typename TMac, typename TMtl, typename TGpio, typename TRcc,
          typename TSysclk, typename TUsart>
Board<TMac, TMtl, TGpio, TRcc, TSysclk, TUsart>& get_board();

using HwBoard = Board<StmH7::StEthMac, StmH7::StEthMtl, StmH7::StGpio,
                      StmH7::StRcc, StmH7::StSysclk, StmH7::StUsart>;

HwBoard& get_hw();

}  // namespace EoT
