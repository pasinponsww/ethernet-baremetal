#pragma once
#include "gpio.h"
#include "mcu_support/stm32/h7xx/stm32h723xx.h"
#include "utils/reg_helpers.h"

namespace Eot
{
namespace StmH7
{

class StGpio : EoT::Gpio<StGpio>
{
public:
    /**
     * @brief Constructor for gpio pin
     * @param port base addr for port (use PORT macros)
     * @param pin_num pin number
     * @param config ptr to gpio config struct
     */

    StGpio(GPIO_TypeDef* const port, uint8_t pin_num,
           StGpioParams* const config)
        : port_addr(port), pin_number(pin_num), params(config)
    {
        // How should I handle invalid pin or port numbers?

        // Set the config registers based on the config params

        // GPIOx_MODER
        SetReg(&(port->MODER), static_cast<uint32_t>(config->mode), pin_num * 2,
               2);

        // GPIOx_OTYPER
        SetReg(&(port->OTYPER), static_cast<uint32_t>(config->otype), pin_num,
               1);

        // GPIOx_OSPEEDR
        SetReg(&(port->OSPEEDR), static_cast<uint32_t>(config->ospeed),
               pin_num * 2, 2);

        // GPIOx_PUPDR
        SetReg(&(port->PUPDR), static_cast<uint32_t>(config->pupdr),
               pin_num * 2, 2);

        // Alternate functions
        // GPIOx_AFRL
        SetReg(&(port->AFR[pin_num / 8]), static_cast<uint32_t>(config->af),
               (pin_num % 8) * 4, 4);
    }

    /**
     * @brief 
     * @param value 1 or 0
     */

    void set(uint8_t value)
    {
    }

    /**
     * @brief returns value of pin
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
    // tells us which port/pin we are working with
    GPIO_TypeDef* const port_addr{GPIOA};
    uint8_t pin_number{0};

    // Config parameters
    StGpioParams* const params{nullptr};
};

enum class MODER : uint8_t
{
    INPUT_MODE = 0,
    OUTPUT_MODE = 1,
    ALTERNATE_FUNCTION_DRAIN = 2,
    ANALOG_MODE = 3
};

enum class OTYPE : uint8_t
{
    PUSH_PULL = 0,
    OPEN_DRAIN = 1
};

enum class OSPEED : uint8_t
{
    LOW_SPEED = 0,
    MED_SPEED = 1,
    HIGH_SPEED = 2,
    VERY_HIGH_SPEED = 3
};

enum class PUPDR : uint8_t
{
    NO_PU_PD = 0,
    PULL_UP = 1,
    PULL_DOWN = 2,
};

enum class AF : uint8_t
{
    AF0 = 0,
    AF1 = 1,
    AF2 = 2,
    AF3 = 3,
    AF4 = 4,
    AF5 = 5,
    AF6 = 6,
    AF7 = 7,
    AF8 = 8,
    AF9 = 9,
    AF10 = 10,
    AF11 = 11,
    AF12 = 12,
    AF13 = 13,
    AF14 = 14,
    AF15 = 15
};

struct StGpioParams
{
    MODER mode;
    OTYPE otype;
    OSPEED ospeed;
    PUPDR pupdr;
    AF af;
};

// explicit StGpio(StGpioParams);

// using namespace StmH7

// // StGpio::StGpioPrams led_config =
// //     MODER::INPUTMODE, OTYPE::PUSH_PULL,
// //     spdasplpdlwpd
// // };

}  // namespace StmH7
}  // namespace Eot
