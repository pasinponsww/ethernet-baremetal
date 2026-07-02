#pragma once
#include "gpio.h"
#include "sysclk.h"
#include "usart.h"

namespace EoT
{

template <typename TGpio, typename TClk, typename TUsart>
struct Board
{
    Gpio<TGpio>& rx;
    Gpio<TGpio>& tx;
    Usart<TUsart>& usart;
    Sysclk<TClk>& clock;
};

bool board_init(void);

template <typename TGpio, typename TClk, typename TUsart>
Board<TGpio, TClk, TUsart>& get_board(void);

}  // namespace EoT
