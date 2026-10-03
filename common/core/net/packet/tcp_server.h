/**
 * @file tcp_server.h
 * @brief Generic TCP server on the lwIP raw API, framed by a PacketProtocol
 * @note Accepts connections, reassembles the byte stream into frames using
 *       TProtocol::frame_size, and answers each with TProtocol::handle.
 *       Runs entirely from lwIP callbacks, i.e. from NetIf::poll() on the
 *       superloop; nothing blocks. The raw API is the only option since
 *       NETCONN/SOCKET need an RTOS.
 *
 *       TCP is a byte stream, so one segment may carry half a frame or
 *       several. Each connection owns a reassembly buffer of one frame,
 *       and the protocol decides where each frame ends.
 *
 *       Resources, all fixed at compile time:
 *         kMaxConnections * (2 * TProtocol::kMaxFrameSize + ~16) bytes of
 *         reassembly + response state (one response buffer per connection,
 *         not shared, so a response stuck retrying on one connection can
 *         never be overwritten by another's). No heap. lwIP pcbs come
 *         from MEMP_NUM_TCP_PCB.
 *
 *       Ownership: holds the protocol by reference (must outlive this).
 *       Owns the listening pcb and every accepted pcb from accept until
 *       release; lwIP owns the pbufs it passes to on_recv, which frees
 *       them on every path. on_error is the one case where lwIP has
 *       already freed the pcb before calling us.
 *
 * @author Bex Saw
 */

#pragma once
#include <array>
#include <cstdint>
#include "lwip/err.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "packet_protocol.h"

namespace EoT::Net
{

/**
 * @brief Connection-level counters
 * @param accepted Connections accepted since start()
 * @param refused Connections refused because every slot was busy
 * @param closed Connections closed, by either end or the idle timeout
 * @param requests Frames handed to the protocol
 * @param dropped Frames the protocol rejected as Invalid (connection
 *        dropped with them)
 * @param send_failures Responses that could not be queued to TCP
 */
struct TcpStats
{
    uint32_t accepted{0U};
    uint32_t refused{0U};
    uint32_t closed{0U};
    uint32_t requests{0U};
    uint32_t dropped{0U};
    uint32_t send_failures{0U};
};

/**
 * @tparam TProtocol The PacketProtocol implementation to serve
 * @tparam kMaxConnections Simultaneous client slots. Each costs one
 *         TProtocol::kMaxFrameSize reassembly buffer, and
 *         MEMP_NUM_TCP_PCB in lwipopts.h must cover them all. uint8_t
 *         because that is what tcp_listen_with_backlog takes
 */
template <typename TProtocol, uint8_t kMaxConnections = 4U>
class TcpServer
{
public:
    /**
     * @brief What the BSP chooses about this listener
     * @param port TCP port to listen on
     * @param poll_interval_ticks How often lwIP calls the idle poll, in
     *        units of the 500 ms TCP coarse timer. 8 = every 4 s
     * @param max_idle_polls Idle polls before a silent connection is
     *        dropped. 15 * 4 s = one minute; a client that vanished
     *        without a FIN would otherwise hold its slot forever
     */
    struct Config
    {
        uint16_t port{TProtocol::kDefaultPort};
        uint8_t poll_interval_ticks{8U};
        uint8_t max_idle_polls{15U};
    };

    explicit TcpServer(PacketProtocol<TProtocol>& protocol) : protocol(protocol)
    {
    }

    // lwIP's listen pcb and every accepted pcb cache a raw pointer into
    // this object (tcp_arg(listen_pcb, this) / tcp_arg(new_pcb, &connection)
    // where `connection` lives in this object's own `connections` array). A
    // move or copy after start() would leave those pcbs pointing at a
    // stale address.
    TcpServer(const TcpServer&) = delete;
    TcpServer& operator=(const TcpServer&) = delete;
    TcpServer(TcpServer&&) = delete;
    TcpServer& operator=(TcpServer&&) = delete;

    /**
     * @brief Bind the listener and start accepting
     * @return true if the listener was bound
     * @note Call after NetIf::init(). Works with the link down.
     */
    bool start(const Config& config = {});

    /**
     * @brief Close the listener and every open connection. Idempotent
     */
    void stop();

    bool is_listening() const
    {
        return listen_pcb != nullptr;
    }

    const TcpStats& statistics() const
    {
        return stats;
    }

private:
    /**
     * @brief One client connection and its reassembly state
     * @param pcb The lwIP connection, null when the slot is free
     * @param owner Back-pointer; lwIP callbacks carry a single void*
     * @param pending Bytes of a partially received frame in `buffer`
     * @param idle_polls Consecutive tcp_poll callbacks with no traffic
     * @param buffer Reassembly buffer, one full frame
     * @param response Response staging, one per connection (not shared):
     *        a response that hits ERR_MEM on tcp_write() has to survive
     *        until on_sent() retries it, which a buffer shared across
     *        connections could not safely do
     * @param response_length Bytes of `response` still waiting to be
     *        queued; 0 means nothing outstanding
     */
    struct Connection
    {
        tcp_pcb* pcb{nullptr};
        TcpServer* owner{nullptr};
        uint16_t pending{0U};
        uint8_t idle_polls{0U};
        std::array<uint8_t, TProtocol::kMaxFrameSize> buffer{};
        std::array<uint8_t, TProtocol::kMaxFrameSize> response{};
        uint16_t response_length{0U};
    };

    // lwIP raw-API callbacks; static because lwIP takes C function
    // pointers. Each recovers its object from the tcp_arg() void*.
    static err_t on_accept(void* arg, tcp_pcb* new_pcb, err_t err);
    static err_t on_recv(void* arg, tcp_pcb* pcb, pbuf* p, err_t err);
    static err_t on_sent(void* arg, tcp_pcb* pcb, u16_t len);
    static err_t on_poll(void* arg, tcp_pcb* pcb);
    static void on_error(void* arg, err_t err);

    Connection* find_free_slot();
    void release(Connection& connection, bool close_pcb);

    /**
     * @brief Consume every complete frame in the reassembly buffer
     * @note Loops because one segment can carry several pipelined frames.
     */
    void drain(Connection& connection);

    /**
     * @brief Hand one complete frame to the protocol and queue its response
     * @param connection The originating connection
     * @param frame_length Frame size, as reported by frame_size
     */
    void handle_frame(Connection& connection, uint16_t frame_length);

    /**
     * @brief tcp_write() connection.response, retrying what ERR_MEM left behind
     * @note Called both right after handle_frame() builds a response and
     *       from on_sent() once lwIP reports buffer space freed up. Leaves
     *       response_length untouched (non-zero) on another ERR_MEM so the
     *       next on_sent() tries again.
     */
    void send_response(Connection& connection);

    PacketProtocol<TProtocol>& protocol;
    Config config{};
    tcp_pcb* listen_pcb{nullptr};
    TcpStats stats{};
    std::array<Connection, kMaxConnections> connections{};
};

}  // namespace EoT::Net

#include "tcp_server_impl.h"
