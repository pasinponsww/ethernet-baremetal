#include "board.h"

#include "st_eth_dma.h"
#include "st_gpio.h"
#include "st_rcc.h"
#include "st_sysclk.h"
#include "st_usart.h"

namespace EoT::StmH7
{
    
static constexpr uint32_t kUsartClockHz{64'000'000};

// RCC
StRcc rcc{};

// USART3 GPIO settings
StGpioSettings uart_io_settings{
    MODER::ALTERNATE_FUNCTION_MODE,
    OTYPE::PUSH_PULL,
    OSPEED::LOW_SPEED,
    PUPDR::NO_PU_PD,
    AF::AF7,
};

// USART3
StUsartSettings usart_params{
    USART3, 115200, kUsartClockHz, true, OversamplingMode::OS_16,
};

StUsart usart{&usart_params};

// USART3 pins: PD8 TX, PD9 RX
StGpio tx{GPIOD, 8, &uart_io_settings};
StGpio rx{GPIOD, 9, &uart_io_settings};

// Enables the SysTick tick that Utils::delay_ms() polls -- without this,
// delay_ms() spins forever (see delay.cc: g_ms_ticks is only ever
// incremented from the SysTick ISR that HAL_Init() arms).
StSysclk clock{Configuration::DEFAULT_HSI_64MHz};

// Ethernet DMA Descriptor setup
// TxDescriptorConfig tx_desc_config{
//     .buff1_addr(),
//     .buff1_len(),
//     .buff1_addr(nullptr)
//     .buff2_len(0)
// };


// Ethernet DMA Settings
TxDescriptorManager<kTxRingSize> tx_desc_manager;
RxDescriptorManager<kRxRingSize> rx_desc_manager;

StEthDmaCtrlSettings eth_dma_ctrl_settings{
    .fixed_burst_length{true},
    .mixed_burst{false},
    .address_aligned_beats{false},
    .rebuild_incrx_burst{false},

    // ETH_DMACCR Reg
    .descriptor_skip_length{0},
    .pblx8_mode{false},
    .maximum_segment_size{0},

    // ETH_DMACTXCR Reg
    .tx_burst_length{DmaBurstLength::beats1},
    .tcp_seg_en{false},
    .osp{false},

    // ETH_DMACRXCR Reg
    .rx_burst_length{DmaBurstLength::beats1}};

// There's a lot of redundancy here that needs to be cleaned up
StEthDmaSettings eth_dma_settings{
    .rx_head{nullptr},  
    .rx_tail{nullptr},
    .tx_head{nullptr},
    .rx_length{kRxRingSize},
    .tx_length{kTxRingSize},
    .rx_buff_size{40},  // must be multiple of 4
    .base_addr{ETH},
    .options{&eth_dma_ctrl_settings}
};

StEthDma<kTxRingSize, kRxRingSize> eth_dma{
    eth_dma_settings, 
    tx_desc_manager,
    rx_desc_manager};

// The one and only board instance
Board<StEthDma<kTxRingSize, kRxRingSize>, StGpio, StRcc, StSysclk, StUsart>
    board{eth_dma, tx, rx, rcc, clock, usart};

}  // namespace EoT::StmH7

namespace EoT
{
bool board_init()
{
    bool result = true;

    // GPIO clocks
    result &= StmH7::rcc.enable_gpio_clock(StmH7::GpioPort::D);  // USART3 TX/RX

    // Peripheral clocks
    result &= StmH7::rcc.enable_uart_clock(StmH7::UartId::U3);
    result &= StmH7::rcc.enable_eth_clock(StmH7::EthernetClock::Tx);
    result &= StmH7::rcc.enable_eth_clock(StmH7::EthernetClock::Rx);

    // GPIO init
    result &= StmH7::tx.init();
    result &= StmH7::rx.init();

    // Peripheral init
    result &= StmH7::clock.init();
    result &= StmH7::usart.init();

    // DMA init
    result &= StmH7::eth_dma.init();

    return result;
}

template <>
Board<StmH7::StEthDma<kTxRingSize, kRxRingSize>, StmH7::StGpio, StmH7::StRcc,
      StmH7::StSysclk, StmH7::StUsart>&
get_board<StmH7::StEthDma<kTxRingSize, kRxRingSize>, StmH7::StGpio,
          StmH7::StRcc, StmH7::StSysclk, StmH7::StUsart>()
{
    return StmH7::board;
}

HwBoard& get_hw()
{
    return get_board<StmH7::StEthDma<kTxRingSize, kRxRingSize>, StmH7::StGpio, StmH7::StRcc,
                     StmH7::StSysclk, StmH7::StUsart>();
}

}  // namespace EoT