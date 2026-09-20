/**
 * @file packet_protocol.h
 * @brief Generic request/response packet protocol CRTP facade
 * @note The seam between a transport (TcpServer<T>, UdpServer<T>) and a
 *       protocol (Modbus::Protocol, ...). The transport owns lwIP, sockets
 *       and buffers; the protocol owns the bytes. Two operations cross the
 *       seam:
 *
 *         frame_size  where does one frame end in this byte buffer? This
 *                     is what lets a stream transport reassemble, and a
 *                     datagram transport validate
 *         handle      turn one complete frame into one response
 *
 *       Widths: lengths are uint16_t, not size_t. Every buffer length lwIP
 *       exposes is u16_t (pbuf tot_len, tcp_write len, udp payload), and
 *       T::kMaxFrameSize bounds every buffer a transport allocates, so a
 *       wider type would carry no information and just be narrowed at the
 *       lwIP boundary. A protocol whose frames could exceed 65535 bytes
 *       has no place on this stack.
 *
 *       Ownership: a transport holds its PacketProtocol<T> by reference
 *       and never copies or frees it. The protocol must outlive every
 *       transport constructed over it.
 *
 *       Same CRTP + concept + static_assert shape as Ethernet<T>: the
 *       concept states the contract, the static_assert in the constructor
 *       reports a violation at the point of instantiation.
 *
 * @author Bex Saw
 */

#pragma once
#include <concepts>
#include <cstdint>
#include <span>

namespace EoT::Net
{

/**
 * @brief Result of PacketProtocol::frame_size
 *        Complete   -> a full frame is present; `length` holds its size
 *        Incomplete -> more bytes are needed. Normal on a stream transport
 *                      (wait for more); an error on a datagram transport
 *        Invalid    -> the bytes are not this protocol at all. A stream
 *                      transport has no delimiter to resync on and drops
 *                      the connection
 */
enum class FrameStatus : uint8_t
{
    Complete = 0,
    Incomplete,
    Invalid
};

// clang-format off
template <typename T>
concept PacketProtocolReq = requires(
    T& protocol,
    std::span<const uint8_t> rx,
    std::span<uint8_t> tx,
    uint16_t& length)
{
    { T::kMaxFrameSize } -> std::convertible_to<uint16_t>;
    { T::kDefaultPort } -> std::convertible_to<uint16_t>;
    { protocol.frame_size(rx, length) } -> std::same_as<FrameStatus>;
    { protocol.handle(rx, tx) } -> std::same_as<uint16_t>;
};
// clang-format on

template <typename T>
class PacketProtocol
{
public:
    /**
     * @brief constructor which uses a static_assert as normal concept syntax
     *        doesn't work
     */
    PacketProtocol()
    {
        static_assert(PacketProtocolReq<T> &&
                      std::derived_from<T, PacketProtocol<T>>);
    }

    /**
     * @brief Find the end of the first frame in `bytes`
     * @param bytes Received bytes, first byte of a frame at offset 0
     * @param[out] length Frame size in bytes, set only on Complete; never
     *        more than T::kMaxFrameSize
     * @note Must not modify anything: a stream transport calls this
     *       repeatedly on the same growing buffer.
     */
    FrameStatus frame_size(std::span<const uint8_t> bytes, uint16_t& length)
    {
        return self().frame_size(bytes, length);
    }

    /**
     * @brief Turn one complete frame into one response
     * @param frame Exactly one frame, as sized by frame_size
     * @param[out] response Destination, T::kMaxFrameSize bytes
     * @return Response length, or 0 to send nothing. The transport sends
     *         exactly this many bytes from `response`, so it is the
     *         protocol's statement of how much of the buffer it owns
     * @note Runs from a lwIP callback on the superloop: never blocks.
     */
    uint16_t handle(std::span<const uint8_t> frame, std::span<uint8_t> response)
    {
        return self().handle(frame, response);
    }

private:
    T& self()
    {
        return static_cast<T&>(*this);
    }
};

}  // namespace EoT::Net
