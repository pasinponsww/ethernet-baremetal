#pragma once
#include "gpio.h"
#include "st_gpio.h"
#include "st_rcc.h"
#include "st_sysclk.h"
#include "sysclk.h"

namespace EoT
{

template <typename TGpio, typename TClk, typename TRcc>
struct Board
{
    Gpio<TGpio>& led1;
    Gpio<TGpio>& led2;
    Gpio<TGpio>& led3;
    Sysclk<TClk>& clock;
    StmH7::Rcc<TRcc>& rcc;
};

bool board_init();

template <typename TGpio, typename TClk, typename TRcc>
Board<TGpio, TClk, TRcc>& get_board();

// StmH7 is now visible as EoT::StmH7 from the includes above
using HwBoard = Board<StmH7::StGpio, StmH7::StSysclk, StmH7::StRcc>;

// From now on we going to use this instance of the board,
// so we can use it in the main.cc without knowing the template parameters
HwBoard& get_hw();

}  // namespace EoT