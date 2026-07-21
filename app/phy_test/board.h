#pragma once
#include "eth_mdio.h"
#include "gpio.h"
#include "lan8742.h"
#include "st_eth_mdio.h"
#include "st_gpio.h"
#include "st_rcc.h"
#include "st_usart.h"
#include "usart.h"

namespace EoT
{

template <typename TEthMdio, typename TGpio, typename TRcc, typename TUsart>
struct Board
{
    EthMdio<TEthMdio>& eth_mdio;
    Gpio<TGpio>& tx;
    Gpio<TGpio>& rx;
    TRcc& rcc;
    Usart<TUsart>& usart;
    Lan8742<TEthMdio>& phy;
};

bool board_init();

template <typename TEthMdio, typename TGpio, typename TRcc, typename TUsart>
Board<TEthMdio, TGpio, TRcc, TUsart>& get_board();

using HwBoard =
    Board<StmH7::StEthMdio, StmH7::StGpio, StmH7::StRcc, StmH7::StUsart>;

HwBoard& get_hw();

}  // namespace EoT