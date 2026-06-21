/**
 * @file st_gpio.h
 * @author Farhaan Khan
 */

#pragma once
#include "gpio.h"
#include "reg_helpers.h"
#include "stm32h723xx.h"

namespace EoT::StmH7
{
constexpr int MAX_NUM_PINS{16};

// Config enums
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

struct StGpioSettings
{
    MODER mode;
    OTYPE otype;
    OSPEED ospeed;
    PUPDR pupdr;
    AF af;
};

class StGpio : public EoT::Gpio<StGpio>
{
public:
    /**
     * @brief Constructor for gpio pin
     * @param port base addr for port (use PORT macros)
     * @param pin_num pin number
     * @param config ptr to gpio config struct
     */
    StGpio(GPIO_TypeDef* const port, uint8_t pin_num,
           StGpioSettings* const config);

    /**
     * @brief configures gpio pin
     */
    bool init();

    /**
     * @brief sets a gpio pin to 1 or 0
     * @param value 1 or 0
     */
    bool set(uint8_t value);

    /**
     * @brief returns value of gpio pin
     */
    bool get() const;

    /**
     * @brief toggles pin
     */
    bool toggle();

private:
    // tells us which port/pin we are working with
    GPIO_TypeDef* const port_addr{nullptr};
    const uint8_t pin_number{0};

    // Config parameters
    StGpioSettings* const settings{nullptr};
};

}  // namespace EoT::StmH7
