#pragma once
#include <span>
#include <vector>
#include "crc.h"
#include "gpio.h"
#include "st_crc.h"
#include "st_gpio.h"
#include "st_sysclk.h"
#include "st_usart.h"
#include "sysclk.h"

namespace EoT
{

template <typename TGpio, typename TClk, typename TCrc, typename TUsart>
struct Board
{
    Gpio<TGpio>& tx;
    Gpio<TGpio>& rx;
    Sysclk<TClk>& clock;
    Crc<TCrc>& crc;
    Usart<TUsart>& usart;
};

bool board_init();

template <typename TGpio, typename TClk, typename TCrc, typename TUsart>
Board<TGpio, TClk, TCrc, TUsart>& get_board();

// StmH7 is now visible as EoT::StmH7 from the includes above
using HwBoard =
    Board<StmH7::StGpio, StmH7::StSysclk, StmH7::StCrc, StmH7::StUsart>;

// From now on we going to use this instance of the board,
// so we can use it in the main.cc without knowing the template parameters
HwBoard& get_hw();

// Simulate noise in the data by randomly dropping or flipping bits
std::vector<uint8_t> simulate_noise(std::span<const uint8_t> data,
                                    double dropProb, double flipProb);

}  // namespace EoT