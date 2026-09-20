/**
 * @file udp_server_impl.h
 * @brief Template implementation for EoT::Net::UdpServer<TProtocol>
 * @note Included from the bottom of udp_server.h. Not meant to be included
 *       directly. One datagram in, one datagram out.
 *
 * @author Bex Saw
 */

#pragma once

namespace EoT::Net
{

template <typename TProtocol>
bool UdpServer<TProtocol>::start(const Config& new_config)
{
    if (pcb != nullptr)
    {
        return true;
    }

    config = new_config;

    udp_pcb* new_pcb{udp_new()};
    if (new_pcb == nullptr)
    {
        return false;
    }

    // IP_ANY_TYPE: with DHCP there is no address to bind to yet.
    if (udp_bind(new_pcb, IP_ANY_TYPE, config.port) != ERR_OK)
    {
        udp_remove(new_pcb);
        return false;
    }

    pcb = new_pcb;
    udp_recv(pcb, &UdpServer::on_recv, this);

    return true;
}

template <typename TProtocol>
void UdpServer<TProtocol>::stop()
{
    if (pcb == nullptr)
    {
        return;
    }

    udp_recv(pcb, nullptr, nullptr);
    udp_remove(pcb);
    pcb = nullptr;
}

template <typename TProtocol>
void UdpServer<TProtocol>::on_recv(void* arg, udp_pcb* upcb, pbuf* p,
                                   const ip_addr_t* addr, u16_t port)
{
    auto* self{static_cast<UdpServer*>(arg)};

    if (p == nullptr)
    {
        return;
    }

    // lwIP hands ownership of the pbuf to this callback; every early
    // return below frees it.
    if (self == nullptr || upcb == nullptr || addr == nullptr)
    {
        pbuf_free(p);
        return;
    }

    ++self->stats.received;

    // Larger than a maximal frame is not this protocol.
    if (p->tot_len == 0U || p->tot_len > TProtocol::kMaxFrameSize)
    {
        ++self->stats.dropped;
        pbuf_free(p);
        return;
    }

    // Flatten a possible pbuf chain into one contiguous buffer.
    const u16_t copied{
        pbuf_copy_partial(p, self->rx_buffer.data(), p->tot_len, 0U)};

    // Done with lwIP's buffer; freed before dispatch so it is released
    // even if that bails out.
    pbuf_free(p);

    if (copied == 0U)
    {
        ++self->stats.dropped;
        return;
    }

    const std::span<const uint8_t> datagram{self->rx_buffer.data(), copied};

    // On a datagram transport Incomplete is not "wait for more", it is a
    // truncated request. Only a Complete frame that fits what arrived is
    // handed on.
    uint16_t frame_length{0U};
    const FrameStatus status{self->protocol.frame_size(datagram, frame_length)};

    if (status != FrameStatus::Complete || frame_length == 0U ||
        frame_length > copied)
    {
        ++self->stats.dropped;
        return;
    }

    const uint16_t response_size{
        self->protocol.handle(datagram.first(frame_length), self->tx_buffer)};

    ++self->stats.requests;

    if (response_size == 0U || response_size > self->tx_buffer.size())
    {
        return;
    }

    // PBUF_RAM so the payload lives in the pbuf and tx_buffer can be reused
    // immediately; udp_sendto queues the pbuf, it does not copy.
    pbuf* out{pbuf_alloc(PBUF_TRANSPORT, response_size, PBUF_RAM)};
    if (out == nullptr)
    {
        ++self->stats.send_failures;
        return;
    }

    if (pbuf_take(out, self->tx_buffer.data(), response_size) != ERR_OK)
    {
        pbuf_free(out);
        ++self->stats.send_failures;
        return;
    }

    // Reply to the request's source: a UDP server has no connection and
    // may be serving several clients at once.
    if (udp_sendto(upcb, out, addr, port) != ERR_OK)
    {
        ++self->stats.send_failures;
    }

    pbuf_free(out);
}

}  // namespace EoT::Net
