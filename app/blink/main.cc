#include <array>
#include <cstddef>
#include <functional>
#include "board.h"
#include "st_gpio.h"

using namespace EoT;

int main(int argc, char* argv[])
{
    board_init();
    Board<StmH7::StGpio>& hw = get_board<StmH7::StGpio>();

    std::array<std::reference_wrapper<Gpio<StmH7::StGpio>>, 3> leds{
        std::ref(hw.led1), std::ref(hw.led2), std::ref(hw.led3)};

    // Repeat 100 times
    for (size_t i = 0; i < 100; ++i)
    {
        for (size_t j = 0; j < leds.size(); ++j)
        {
            leds[j].get().set(1);
        }

        // Delay
        for (volatile size_t j = 0; j < 800000; j++)
        {
        }

        for (size_t j = 0; j < leds.size(); ++j)
        {
            leds[j].get().set(0);
        }

        // Delay
        for (volatile size_t j = 0; j < 800000; j++)
        {
        }
    }

    return 0;
}