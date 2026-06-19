#pragma once
#include "gpio.h"

namespace Eot
{
namespace StmH7
{

class StGpio : EoT::Gpio<StGpio>
{
public:
    /**
     * @brief 
     * @param 1 or 0
     */
    void set(uint8_t value)
    {
    }

    /**
     * @brief returns value of 
     */
    uint8_t get()
    {
    }

    /**
     * @brief toggles pin
     */
    void toggle()
    {
    }

private:
    // parameters:
    // base_addr (tells us which pin we are working with)
    //
};

}  // namespace StmH7
}  // namespace Eot
