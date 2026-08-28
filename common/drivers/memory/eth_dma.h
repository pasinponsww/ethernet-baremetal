/**
 * @file dma.h
 * @brief Ethernet DMA driver interface
 * @author Farhaan Khan
 * @date 7/14/2026
 */

#pragma once
#include <concepts>
#include <cstdint>

namespace EoT
{

/** 
* @brief Config struct for tx descriptor,
* can be extended in future to support more functionality.
*/
struct TxDescriptorConfig
{
    uint32_t buff1_addr{0};
    uint32_t buff1_len{0};
    uint32_t buff2_addr{0};
    uint32_t buff2_len{0};
    bool is_start_of_packet{false};
    bool is_end_of_packet{true};
};

/** 
* @brief Config struct for rx descriptor,
* can be extended in future to support more functionality.
*/
struct RxDescriptorConfig
{
    uint32_t buff1_addr{0};
    uint32_t buff2_addr{0};
};

// clang-format off
template <typename T>
concept EthDmaReq = requires(
    T t, 
    uint8_t num_descriptors, 
    TxDescriptorConfig& tx_desc_config,
    RxDescriptorConfig& rx_desc_config)
{
    { t.init() } -> std::same_as<bool>;
    { t.insert_tx_desc(tx_desc_config) } -> std::same_as<bool>;
    { t.insert_rx_desc(rx_desc_config) } -> std::same_as<bool>;
    { t.send_packet() } -> std::same_as<bool>;
    { t.receive(num_descriptors) } -> std::same_as<bool>;
};

// clang-format on

template <typename T>
class EthDma
{
public:
    /**
    * @brief Constructor for the EthDma class
    * @note This constructor is protected to prevent direct instantiation of the EthDma class
    */
    EthDma()
    {
        static_assert(EthDmaReq<T> && std::derived_from<T, EthDma>);
    }

    /**
    * @brief Initialize Ethernet DMA
    * @return status of initialize operation
    */
    bool init()
    {
        return self().init();
    }

    /**
     * @brief Add tx descriptor to tx ring
     * @return status of operation
     */
    bool insert_tx_desc(TxDescriptorConfig& config)
    {
        return self().insert_tx_desc(config);
    }

     /**
     * @brief Add rx descriptor to rx ring
     * @return status of operation
     */
    bool insert_rx_desc(RxDescriptorConfig& config)
    {
        return self().insert_rx_desc(config);
    }

    /**
     * @brief Send ethernet packet (or all data in descriptors if none is marked as being the end of a packet)
     * @return status of operation
     */
    bool send_packet()
    {
        return self().send_packet();
    }

    /**
     * @brief Receive ethernet data
     * @param num_descriptors number of descriptors to store data into
     * @return status of operation
     */
    bool receive(uint8_t num_descriptors)
    {
        return self().receive(num_descriptors);
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

}  // namespace EoT