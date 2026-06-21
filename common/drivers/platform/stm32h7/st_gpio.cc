#include "st_gpio.h"

using namespace EoT::StmH7;

StGpio::StGpio(GPIO_TypeDef* const port, uint8_t pin_num,
               StGpioParams* const config)
    : port_addr(port), pin_number(pin_num), params(config)
{
}

bool StGpio::init()
{
    // Handle invalid pin number
    if (pin_number < 0 || pin_number >= MAX_NUM_PINS)
        return false;

    auto addr = reinterpret_cast<std::uintptr_t>(port_addr);

    // Handle invalid port
    if (addr % 1024 != 0 || addr < GPIOA_BASE || addr > GPIOK_BASE)
        return false;

    // GPIOx_MODER
    SetReg(&(port_addr->MODER), static_cast<uint32_t>(params->mode),
           pin_number * 2, 2);

    // GPIOx_OTYPER
    SetReg(&(port_addr->OTYPER), static_cast<uint32_t>(params->otype),
           pin_number, 1);

    // GPIOx_OSPEEDR
    SetReg(&(port_addr->OSPEEDR), static_cast<uint32_t>(params->ospeed),
           pin_number * 2, 2);

    // GPIOx_PUPDR
    SetReg(&(port_addr->PUPDR), static_cast<uint32_t>(params->pupdr),
           pin_number * 2, 2);

    // Alternate functions
    // GPIOx_AFRL
    SetReg(&(port_addr->AFR[pin_number / 8]), static_cast<uint32_t>(params->af),
           (pin_number % 8) * 4, 4);

    return true;
}

void StGpio::set(uint8_t value)
{
    SetReg(&(port_addr->ODR), static_cast<uint32_t>(value), pin_number, 1);
}

uint8_t StGpio::get()
{
    return ((port_addr->IDR >> pin_number) && uint32_t{0x01});
}

void StGpio::toggle()
{
    uint32_t bit = ((port_addr->IDR >> pin_number) && uint32_t{0x01});
    SetReg(&(port_addr->ODR), bit ^ 1, pin_number, 1);
}
