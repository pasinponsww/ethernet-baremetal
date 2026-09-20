/**
 * @file tcp_server_impl.h
 * @brief Template implementation for EoT::Net::TcpServer<TProtocol, N>
 * @note Included from the bottom of tcp_server.h (same convention as
 *       net.h / net_impl.h). Not meant to be included directly.
 *
 *       The rule throughout: the protocol's frame_size, and nothing else,
 *       says where a frame ends.
 *
 * @author Bex Saw
 */

#pragma once

#include <cstring>  // memmove

namespace EoT::Net
{

template <typename TProtocol, uint8_t kMaxConnections>
bool TcpServer<TProtocol, kMaxConnections>::start(const Config& new_config)
{
    if (listen_pcb != nullptr)
    {
        return true;
    }

    config = new_config;

    tcp_pcb* pcb{tcp_new()};
    if (pcb == nullptr)
    {
        return false;
    }

    // IP_ANY_TYPE: with DHCP there is no address to bind to yet.
    if (tcp_bind(pcb, IP_ANY_TYPE, config.port) != ERR_OK)
    {
        tcp_close(pcb);
        return false;
    }

    // tcp_listen returns a new pcb and frees the original on success.
    tcp_pcb* listener{tcp_listen_with_backlog(pcb, kMaxConnections)};
    if (listener == nullptr)
    {
        tcp_close(pcb);
        return false;
    }

    listen_pcb = listener;

    tcp_arg(listen_pcb, this);
    tcp_accept(listen_pcb, &TcpServer::on_accept);

    return true;
}

template <typename TProtocol, uint8_t kMaxConnections>
void TcpServer<TProtocol, kMaxConnections>::stop()
{
    for (Connection& connection : connections)
    {
        if (connection.pcb != nullptr)
        {
            release(connection, true);
        }
    }

    if (listen_pcb != nullptr)
    {
        tcp_close(listen_pcb);
        listen_pcb = nullptr;
    }
}

template <typename TProtocol, uint8_t kMaxConnections>
typename TcpServer<TProtocol, kMaxConnections>::Connection*
TcpServer<TProtocol, kMaxConnections>::find_free_slot()
{
    for (Connection& connection : connections)
    {
        if (connection.pcb == nullptr)
        {
            return &connection;
        }
    }

    return nullptr;
}

template <typename TProtocol, uint8_t kMaxConnections>
void TcpServer<TProtocol, kMaxConnections>::release(Connection& connection,
                                                    bool close_pcb)
{
    if (connection.pcb != nullptr)
    {
        // close_pcb == false means on_error called us and lwIP has already
        // freed the pcb: it is dangling and must not be touched.
        if (close_pcb)
        {
            // Detach first so no callback fires against a released slot.
            tcp_arg(connection.pcb, nullptr);
            tcp_recv(connection.pcb, nullptr);
            tcp_poll(connection.pcb, nullptr, 0U);
            tcp_err(connection.pcb, nullptr);

            // tcp_close can fail when memory is tight; abort guarantees
            // the pcb is gone rather than leaked.
            if (tcp_close(connection.pcb) != ERR_OK)
            {
                tcp_abort(connection.pcb);
            }
        }

        ++stats.closed;
    }

    connection.pcb = nullptr;
    connection.owner = nullptr;
    connection.pending = 0U;
    connection.idle_polls = 0U;
}

template <typename TProtocol, uint8_t kMaxConnections>
err_t TcpServer<TProtocol, kMaxConnections>::on_accept(void* arg,
                                                       tcp_pcb* new_pcb,
                                                       err_t err)
{
    auto* self{static_cast<TcpServer*>(arg)};

    if (self == nullptr || new_pcb == nullptr || err != ERR_OK)
    {
        return ERR_VAL;
    }

    Connection* connection{self->find_free_slot()};
    if (connection == nullptr)
    {
        // Every slot busy: abort so the client retries, rather than leave
        // it waiting on a connection nobody will read.
        ++self->stats.refused;
        tcp_abort(new_pcb);
        return ERR_ABRT;
    }

    connection->pcb = new_pcb;
    connection->owner = self;
    connection->pending = 0U;
    connection->idle_polls = 0U;

    ++self->stats.accepted;

    // From here the connection, not the server, is the callback argument.
    tcp_arg(new_pcb, connection);
    tcp_recv(new_pcb, &TcpServer::on_recv);
    tcp_err(new_pcb, &TcpServer::on_error);
    tcp_poll(new_pcb, &TcpServer::on_poll, self->config.poll_interval_ticks);

    // Request/response with small payloads: Nagle would only hold a
    // response back waiting for data that is not coming.
    tcp_nagle_disable(new_pcb);

    return ERR_OK;
}

template <typename TProtocol, uint8_t kMaxConnections>
err_t TcpServer<TProtocol, kMaxConnections>::on_recv(void* arg, tcp_pcb* pcb,
                                                     pbuf* p, err_t err)
{
    auto* connection{static_cast<Connection*>(arg)};

    if (connection == nullptr || connection->owner == nullptr)
    {
        // Slot already released; nothing owns this connection.
        if (p != nullptr)
        {
            pbuf_free(p);
        }

        tcp_abort(pcb);
        return ERR_ABRT;
    }

    TcpServer& self{*connection->owner};

    // A null pbuf means the remote end sent FIN.
    if (p == nullptr)
    {
        self.release(*connection, true);
        return ERR_OK;
    }

    if (err != ERR_OK)
    {
        pbuf_free(p);
        self.release(*connection, true);
        return ERR_OK;
    }

    connection->idle_polls = 0U;

    // Every path below either buffers or discards the data, so the receive
    // window can advance up front.
    tcp_recved(pcb, p->tot_len);

    // The reassembly buffer holds one frame but a segment may carry
    // several, so feed it in chunks and drain complete frames between them.
    u16_t offset{0U};

    while (offset < p->tot_len)
    {
        const uint16_t room{static_cast<uint16_t>(TProtocol::kMaxFrameSize -
                                                  connection->pending)};

        if (room == 0U)
        {
            // Buffer full and nothing drainable: the protocol claims a
            // frame larger than its own kMaxFrameSize, so the stream is
            // unframeable.
            ++self.stats.dropped;
            pbuf_free(p);
            self.release(*connection, true);
            return ERR_OK;
        }

        const u16_t remaining{static_cast<u16_t>(p->tot_len - offset)};
        const u16_t chunk{(remaining < room) ? remaining : room};

        pbuf_copy_partial(p, connection->buffer.data() + connection->pending,
                          chunk, offset);

        connection->pending =
            static_cast<uint16_t>(connection->pending + chunk);
        offset = static_cast<u16_t>(offset + chunk);

        self.drain(*connection);

        // drain() may have torn the connection down on an invalid frame.
        if (connection->pcb == nullptr)
        {
            pbuf_free(p);
            return ERR_OK;
        }
    }

    pbuf_free(p);

    return ERR_OK;
}

template <typename TProtocol, uint8_t kMaxConnections>
void TcpServer<TProtocol, kMaxConnections>::drain(Connection& connection)
{
    while (connection.pending > 0U)
    {
        uint16_t frame_length{0U};
        const FrameStatus status{protocol.frame_size(
            std::span<const uint8_t>{connection.buffer.data(),
                                     connection.pending},
            frame_length)};

        if (status == FrameStatus::Incomplete)
        {
            // Header or body still in flight.
            return;
        }

        if (status != FrameStatus::Complete || frame_length == 0U ||
            frame_length > connection.pending)
        {
            // Not this protocol at all. No delimiter to resync on, so drop
            // the connection.
            ++stats.dropped;
            release(connection, true);
            return;
        }

        handle_frame(connection, frame_length);

        // Shift the next frame down to the front. memmove: the ranges
        // overlap whenever frames were pipelined.
        const uint16_t leftover{
            static_cast<uint16_t>(connection.pending - frame_length)};
        if (leftover > 0U)
        {
            std::memmove(connection.buffer.data(),
                         connection.buffer.data() + frame_length, leftover);
        }

        connection.pending = leftover;
    }
}

template <typename TProtocol, uint8_t kMaxConnections>
void TcpServer<TProtocol, kMaxConnections>::handle_frame(Connection& connection,
                                                         uint16_t frame_length)
{
    const std::span<const uint8_t> frame{connection.buffer.data(),
                                         frame_length};

    const uint16_t response_size{protocol.handle(frame, tx_buffer)};

    ++stats.requests;

    if (response_size == 0U || response_size > tx_buffer.size())
    {
        // Protocol chose silence.
        return;
    }

    // TCP_WRITE_FLAG_COPY: tx_buffer is reused by the next frame while
    // lwIP still holds the data until it is acknowledged.
    const err_t written{tcp_write(connection.pcb, tx_buffer.data(),
                                  response_size, TCP_WRITE_FLAG_COPY)};

    if (written != ERR_OK)
    {
        // Out of send buffer. Let the client time out and retry rather
        // than tear down a healthy connection.
        ++stats.send_failures;
        return;
    }

    // Push now rather than waiting for the next lwIP timer tick.
    tcp_output(connection.pcb);
}

template <typename TProtocol, uint8_t kMaxConnections>
err_t TcpServer<TProtocol, kMaxConnections>::on_poll(void* arg, tcp_pcb* pcb)
{
    auto* connection{static_cast<Connection*>(arg)};

    if (connection == nullptr || connection->owner == nullptr)
    {
        tcp_abort(pcb);
        return ERR_ABRT;
    }

    ++connection->idle_polls;

    if (connection->idle_polls >= connection->owner->config.max_idle_polls)
    {
        connection->owner->release(*connection, true);
    }

    return ERR_OK;
}

template <typename TProtocol, uint8_t kMaxConnections>
void TcpServer<TProtocol, kMaxConnections>::on_error(void* arg, err_t err)
{
    (void)err;

    auto* connection{static_cast<Connection*>(arg)};

    if (connection == nullptr || connection->owner == nullptr)
    {
        return;
    }

    // lwIP has already freed the pcb; closing it again would double free.
    connection->owner->release(*connection, false);
}

}  // namespace EoT::Net
