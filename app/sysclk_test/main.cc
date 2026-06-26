#include <array>
#include <cstddef>
#include <functional>
#include "board.h"
#include "delay.h"
#include "st_gpio.h"
#include "st_sysclk.h"

using namespace EoT;

/* This test is a blink but in different config of the sysclk */
int main(int argc, char* argv[])
{
    board_init();
    HwBoard& hw = get_hw();

    std::array<std::reference_wrapper<Gpio<StmH7::StGpio>>, 3> leds{
        std::ref(hw.led1), std::ref(hw.led2), std::ref(hw.led3)};

    // Repeat 100 times
    for (size_t i = 0; i < 100; ++i)
    {
        for (size_t j = 0; j < leds.size(); ++j)
        {
            leds[j].get().set(1);
        }

        Utils::delay_ms(500);

        for (size_t j = 0; j < leds.size(); ++j)
        {
            leds[j].get().set(0);
        }

        Utils::delay_ms(500);
    }

    return 0;
}
