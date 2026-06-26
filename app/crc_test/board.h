#pragma once
#include "gpio.h"
#include "sysclk.h"
#include "st_crc.h"
#include "st_gpio.h"
#include "st_sysclk.h"
#include "sysclk.h"
#include "st_crc.h"

namespace EoT
{

template <typename TGpio, typename TClk>
struct Board
{
    Gpio<TGpio>& led1;
    Gpio<TGpio>& led2;
    Gpio<TGpio>& led3;
    Sysclk<TClk>& clock;
    Crc<StmH7::StCrc>& crc;
};

bool board_init();

template <typename TGpio, typename TClk>
Board<TGpio, TClk>& get_board();

// StmH7 is now visible as EoT::StmH7 from the includes above
using HwBoard = Board<StmH7::StGpio, StmH7::StSysclk>;

// From now on we going to use this instance of the board,
// so we can use it in the main.cc without knowing the template parameters
HwBoard& get_hw();

}  // namespace EoT