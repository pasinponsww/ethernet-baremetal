#include "st_eth_mdio.h"

namespace EoT::StmH7
{

static constexpr uint8_t kMDIO_PHY_ADDR_COUNT{32U};
static constexpr uint8_t kMDIO_REG_ADDR_COUNT{32U};

static inline bool valid_phy(uint8_t phy_addr)
{
    return phy_addr < kMDIO_PHY_ADDR_COUNT;
}

static inline bool valid_reg(uint8_t reg_addr)
{
    return reg_addr < kMDIO_REG_ADDR_COUNT;
}

StEthMdio::StEthMdio(const StEthMdioConfig& config)
    : base_addr(config.base_addr),
      csr_clock_hz(config.csr_clock_hz),
      timeout_us(config.timeout_us)
{
}

EoT::EthMdioStatus StEthMdio::init(const StEthMdioConfig& config)
{

    return EoT::EthMdioStatus::Ok;
}

EoT::EthMdioStatus StEthMdio::read(uint8_t phy_addr, uint8_t reg_addr,
                                   uint16_t& data)
{

    return EoT::EthMdioStatus::Ok;
}

EoT::EthMdioStatus StEthMdio::write(uint8_t phy_addr, uint8_t reg_addr,
                                    uint16_t data)
{

    return EoT::EthMdioStatus::Ok;
}

}  // namespace EoT::StmH7