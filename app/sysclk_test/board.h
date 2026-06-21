#pragma once
#include "gpio.h"
#include "sysclk.h"

namespace EoT
{

/* This test is a blink but in different config of the sysclk */

template <typename TGpio, typename TClk>
struct Board
{
    Gpio<TGpio>& led1;
    Gpio<TGpio>& led2;
    Gpio<TGpio>& led3;
    Sysclk<TClk>& clock;
};

bool board_init(void);

template <typename TGpio, typename TClk>
Board<TGpio, TClk>& get_board(void);

}  // namespace EoT
