#pragma once

#include "eth_dma.h"
#include "gpio.h"
#include "st_eth_dma.h"
#include "st_gpio.h"
#include "st_rcc.h"
#include "st_sysclk.h"
#include "st_usart.h"
#include "usart.h"

namespace EoT
{

constexpr uint16_t kTxRingSize{10};
constexpr uint16_t kRxRingSize{10};
constexpr uint16_t kRxBuffSize{64};

template <typename TEthDma, typename TGpio, typename TRcc, typename TSysclk,
          typename TUsart>
struct Board
{
    EthDma<TEthDma>& eth_dma;
    Gpio<TGpio>& tx;
    Gpio<TGpio>& rx;
    TRcc& rcc;
    TSysclk& clock;
    Usart<TUsart>& usart;
};

bool board_init();

template <typename TEthDma, typename TGpio, typename TRcc, typename TSysclk,
          typename TUsart>
Board<TEthDma, TGpio, TRcc, TSysclk, TUsart>& get_board();

using HwBoard = Board<StmH7::StEthDma<kTxRingSize, kRxRingSize>, StmH7::StGpio,
                      StmH7::StRcc, StmH7::StSysclk, StmH7::StUsart>;

HwBoard& get_hw();

}  // namespace EoT
