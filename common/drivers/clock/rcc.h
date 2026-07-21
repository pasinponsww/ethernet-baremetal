#pragma once

#include <concepts>
#include <cstdint>

namespace EoT::StmH7
{

template <typename T>
concept RccReq = requires(const T& t) {
    {
        t.enable_eth_clock(EthernetClock::Mac)
    } -> std::same_as<bool>;
    {
        t.enable_gpio_clock(GpioPort::A)
    } -> std::same_as<bool>;
    {
        t.enable_uart_clock(UartId::U1)
    } -> std::same_as<bool>;
    {
        t.enable_dma_clock(DmaId::D1)
    } -> std::same_as<bool>;
    {
        t.enable_crc_clock(AhbPeriph::Crc)
    } -> std::same_as<bool>;
    {
        t.get_freq()
    } -> std::same_as<uint32_t>;
};

template <typename T>
class Rcc
{
public:
    bool enable_eth_clock(EthernetClock clock)
    {
        static_assert(RccReq<T>);
        return self().enable_eth_clock(clock);
    }

    bool enable_gpio_clock(GpioPort port)
    {
        static_assert(RccReq<T>);
        return self().enable_gpio_clock(port);
    }

    bool enable_uart_clock(UartId usart)
    {
        static_assert(RccReq<T>);
        return self().enable_uart_clock(usart);
    }

    bool enable_dma_clock(DmaId dma)
    {
        static_assert(RccReq<T>);
        return self().enable_dma_clock(dma);
    }

    bool enable_crc_clock(AhbPeriph peripheral)
    {
        static_assert(RccReq<T>);
        return self().enable_crc_clock(peripheral);
    }

    uint32_t get_freq() const
    {
        static_assert(RccReq<T>);
        return self().get_freq();
    }

private:
    T& self()
    {
        return static_cast<T&>(*this);
    }

    const T& self() const
    {
        return static_cast<const T&>(*this);
    }
};

}  // namespace EoT::StmH7