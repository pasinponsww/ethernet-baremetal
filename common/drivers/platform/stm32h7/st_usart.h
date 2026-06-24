/**
 * @file st_usart.h
 * @author Farhaan Khan
 */

#pragma once
#include "stm32h723xx.h"
#include "usart.h"

namespace EoT::StmH7
{

class StUsart : public EoT::Usart<StUsart>
{
public:
    bool init();

    bool send(const std::span<const uint8_t> data);

    bool receive(uint8_t& byte);

private:
    USART_TypeDef* base_addr{nullptr};
    uint32_t baud_rate{9600};
};

}  // namespace EoT::StmH7