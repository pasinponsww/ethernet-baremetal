#pragma once
#include "gpio.h"

namespace EoT
{

template <typename T>
struct Board
{
    Gpio<T>& led1;
    Gpio<T>& led2;
    Gpio<T>& led3;
};

bool board_init(void);

template <typename T>
Board<T>& get_board(void);

}  // namespace EoT
