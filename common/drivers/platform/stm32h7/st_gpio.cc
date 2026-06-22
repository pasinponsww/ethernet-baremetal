#include "st_gpio.h"

using namespace EoT::StmH7;

static constexpr uint16_t PORT_OFFSET{1024};

StGpio::StGpio(GPIO_TypeDef* const port, uint8_t pin_num,
               StGpioSettings* const config)
    : port_addr(port), pin_number(pin_num), settings(config)
{
}

bool StGpio::init()
{
    // Handle invalid pin number
    if (pin_number < 0 || pin_number >= MAX_NUM_PINS)
        return false;

    auto addr = reinterpret_cast<std::uintptr_t>(port_addr);

    // Handle invalid port
    if (port_addr == nullptr || addr % PORT_OFFSET != 0 || addr < GPIOA_BASE ||
        addr > GPIOK_BASE)
        return false;

    // Enable the GPIO port clock. The RCC->AHB4ENR enable bit index equals the
    // port index (GPIOA = 0, GPIOB = 1, ... GPIOK = 10), which is the port's
    // address offset from GPIOA divided by the per-port spacing.
    uint32_t port_index = (addr - GPIOA_BASE) / PORT_OFFSET;
    SetReg(&(RCC->AHB4ENR), 1, port_index, 1);
    (void)
        RCC->AHB4ENR;  // read-back so the clock is up before we touch the port

    // GPIOx_MODER
    SetReg(&(port_addr->MODER), static_cast<uint32_t>(settings->mode),
           pin_number * 2, 2);

    // GPIOx_OTYPER
    SetReg(&(port_addr->OTYPER), static_cast<uint32_t>(settings->otype),
           pin_number, 1);

    // GPIOx_OSPEEDR
    SetReg(&(port_addr->OSPEEDR), static_cast<uint32_t>(settings->ospeed),
           pin_number * 2, 2);

    // GPIOx_PUPDR
    SetReg(&(port_addr->PUPDR), static_cast<uint32_t>(settings->pupdr),
           pin_number * 2, 2);

    // Alternate functions
    // GPIOx_AFRL
    SetReg(&(port_addr->AFR[pin_number / 8]),
           static_cast<uint32_t>(settings->af), (pin_number % 8) * 4, 4);

    return true;
}

bool StGpio::set(uint8_t value)
{
    bool result{false};
    SetReg(&(port_addr->ODR), static_cast<uint32_t>(value), pin_number, 1);
    result = true;
    return result;
}

bool StGpio::get() const
{
    return ((port_addr->IDR >> pin_number) && uint32_t{0x01});
}

bool StGpio::toggle()
{
    bool result{false};
    uint32_t bit = ((port_addr->IDR >> pin_number) && uint32_t{0x01});
    SetReg(&(port_addr->ODR), bit ^ 1, pin_number, 1);
    result = true;
    return result;
}
