#pragma once

#include "gpio.h"
#include "usart.h"

namespace EoT
{

/**
* @brief Traits for a specific board, including the hardware peripherals that
*        are used to implement the EoT::Ethernet interface.
*        This is a template to allow for different hardware implementations.
*/
template <typename THw>
struct Board
{
    typename THw::Ethernet& eth;
    Gpio<typename THw::Gpio>& tx;
    Gpio<typename THw::Gpio>& rx;
    typename THw::Rcc& rcc;
    typename THw::Sysclk& clock;
    Usart<typename THw::Usart>& usart;
};

}  // namespace EoT
