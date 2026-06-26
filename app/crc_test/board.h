#pragma once
#include "crc.h"
#include "gpio.h"
#include "st_crc.h"
#include "st_gpio.h"
#include "st_sysclk.h"
#include "sysclk.h"
// #include "st_uart.h"
// #include "uart.h"
#include <span>
#include <vector>

namespace EoT
{

template <typename TGpio, typename TClk, typename TCrc>
struct Board
{
    Gpio<TGpio>& tx;
    Gpio<TGpio>& rx;
    Sysclk<TClk>& clock;
    Crc<TCrc>& crc;
    // Uart<TUart>& uart; waiting on Farhaan
};

bool board_init();

template <typename TGpio, typename TClk, typename TCrc>
Board<TGpio, TClk, TCrc>& get_board();

// StmH7 is now visible as EoT::StmH7 from the includes above
using HwBoard = Board<StmH7::StGpio, StmH7::StSysclk, StmH7::StCrc>;

// From now on we going to use this instance of the board,
// so we can use it in the main.cc without knowing the template parameters
HwBoard& get_hw();

// Simulate noise in the data by randomly dropping or flipping bits
std::vector<uint8_t> simulate_noise(std::span<const uint8_t> data,
                                    double dropProb, double flipProb);

}  // namespace EoT