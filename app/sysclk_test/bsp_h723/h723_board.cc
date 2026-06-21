#include "board.h"
#include "st_gpio.h"
#include "st_sysclk.h"

namespace EoT::StmH7
{

// Configure 3 LEDs
StGpioSettings led_settings{MODER::OUTPUT_MODE, OTYPE::PUSH_PULL,
                            OSPEED::LOW_SPEED, PUPDR::NO_PU_PD, AF::AF0};

StGpio led1{GPIOB, 0, &led_settings};
StGpio led2{GPIOE, 1, &led_settings};
StGpio led3{GPIOB, 14, &led_settings};

// Configure SysClk
StSysclk clock{Configuration::DEFAULT_HSI_64MHz};

}  // namespace EoT::StmH7

namespace EoT
{

Board<StmH7::StGpio, StmH7::StSysclk> board{.led1 = StmH7::led1,
                                            .led2 = StmH7::led2,
                                            .led3 = StmH7::led3,
                                            .clock = StmH7::clock};

bool board_init()
{
    bool result = false;

    result = StmH7::clock.init();
    result = StmH7::led1.init();
    result = StmH7::led2.init();
    result = StmH7::led3.init();

    return result;
}

template <>
Board<StmH7::StGpio, StmH7::StSysclk>&
get_board<StmH7::StGpio, StmH7::StSysclk>(void)
{
    return board;
}
}  // namespace EoT