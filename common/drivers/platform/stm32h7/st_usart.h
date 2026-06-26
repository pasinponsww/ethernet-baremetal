/**
 * @file st_usart.h
 * @author Farhaan Khan
 */

#pragma once
#include "stm32h723xx.h"
#include "usart.h"

namespace EoT::StmH7
{

enum class OversamplingMode : uint8_t
{
    OS_16 = 0,
    OS_8 = 1
};

struct StUsartSettings
{
    USART_TypeDef* base_addr{nullptr};
    uint32_t baud_rate{115200};
    uint32_t clk_freq;
    bool fifo_mode{true};
    OversamplingMode sampling_mode{OversamplingMode::OS_16};
};

class StUsart : public EoT::Usart<StUsart>
{
public:
    /**
     * @brief Constructor for USART
     * @param config struct ptr with configuration parameters 
     */
    explicit StUsart(StUsartSettings* config);

    /**
     * @brief Configures USART
     */
    bool init();

    /**
     * @brief Sends array of bytes (polling)
     * @param data array of bytes to be sent
     */
    bool send(const std::span<const uint8_t> data);

    /**
     * @brief Recieves byte (interrupts)
     * @param byte where data is stored
     */
    bool receive(uint8_t& byte);

private:
    StUsartSettings* const params;
};

}  // namespace EoT::StmH7