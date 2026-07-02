#include <random>
#include "board.h"
#include "st_crc.h"
#include "st_gpio.h"
#include "st_sysclk.h"
#include "st_usart.h"

namespace EoT::StmH7
{

// CRC-32/BZIP2
static constexpr uint32_t kDefaultPoly{0x4C11DB7};
static constexpr uint32_t kCrcValue{CRC_INIT_INIT};
static constexpr uint32_t kCrcXOROut{0xFFFFFFFF};

StCrcSettings crc_settings{CrcRevOut::NO_REV, CrcRevIn::NO_REV,
                           CrcPolySize::SIZE_32};
const StCrcParams crc_params{crc_settings, CRC, kCrcValue, kDefaultPoly,
                             kCrcXOROut};

StGpioSettings uart_io_settings{MODER::ALTERNATE_FUNCTION_MODE,
                                OTYPE::PUSH_PULL, OSPEED::LOW_SPEED,
                                PUPDR::NO_PU_PD, AF::AF7};

// Configure USART
StUsartSettings usart_params{USART3, 115200, 64'000'000, true,
                             OversamplingMode::OS_16};

StSysclk clock{Configuration::DEFAULT_HSI_64MHz};

StCrc crc{crc_params};
StUsart usart{&usart_params};
// USART3 is connected to GPIOD pins 8 (TX) and 9 (RX)
StGpio tx{GPIOD, 8, &uart_io_settings};
StGpio rx{GPIOD, 9, &uart_io_settings};

// The one and only board instance
Board<StGpio, StSysclk, StCrc, StUsart> board{tx, rx, clock, crc, usart};

}  // namespace EoT::StmH7

namespace EoT
{

bool board_init()
{
    RCC->AHB4ENR |= RCC_AHB4ENR_GPIODEN;  // GPIOD clock for USART3 TX/RX
    RCC->AHB4ENR |= RCC_AHB4ENR_GPIOBEN | RCC_AHB4ENR_GPIOEEN;  // LED GPIOs
    RCC->APB1LENR |= RCC_APB1LENR_USART3EN;                     // USART3 clock
    RCC->AHB4ENR |= RCC_AHB4ENR_CRCEN;                          // CRC clock

    bool result = true;
    result &= StmH7::crc.init();
    result &= StmH7::clock.init();
    result &= StmH7::tx.init();
    result &= StmH7::rx.init();
    result &= StmH7::usart.init();

    NVIC_SetPriority(USART3_IRQn, 0);
    NVIC_EnableIRQ(USART3_IRQn);

    return result;
}

template <>
Board<StmH7::StGpio, StmH7::StSysclk, StmH7::StCrc, StmH7::StUsart>&
get_board<StmH7::StGpio, StmH7::StSysclk, StmH7::StCrc, StmH7::StUsart>()
{
    return StmH7::board;
}

HwBoard& get_hw()
{
    return get_board<StmH7::StGpio, StmH7::StSysclk, StmH7::StCrc,
                     StmH7::StUsart>();
}

std::vector<uint8_t> simulate_noise(std::span<const uint8_t> data,
                                    double dropProb, double flipProb)
{
    std::mt19937 gen{std::random_device{}()};
    std::uniform_real_distribution<double> dis(0.0, 1.0);
    std::vector<uint8_t> res;
    res.reserve(data.size());

    for (uint8_t b : data)
    {
        if (dis(gen) < dropProb)
            continue;
        for (int i = 0; i < 8; ++i)
            if (dis(gen) < flipProb)
                b ^= (1u << i);
        res.push_back(b);
    }
    return res;
}

}  // namespace EoT