#include "board.h"
#include "st_gpio.h"
#include "st_sysclk.h"
#include "st_usart.h"
#include "delay.h"

using namespace EoT;

int main(int argv, char* argc[])
{
    board_init();
    auto& hw = get_board<StmH7::StGpio, StmH7::StSysclk, StmH7::StUsart>();

    while (1)
    {
        std::array<uint8_t, 20> message{"downers"};
        hw.usart.send(message);
    }

    Utils::delay_ms(5000); 

}