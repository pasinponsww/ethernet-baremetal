# `common/core/net` — lwIP port, link orchestration and packet protocols

Everything above `EoT::Ethernet<T>` and below the application. No RTOS: lwIP
runs in `NO_SYS=1` mode and is driven entirely from `NetIf::poll()` on the
superloop, so only the raw (callback) API exists and nothing in this tree may
block.

> **Status:** the `Ethernet<T>` facade (`common/core/ethernet/ethernet.h`) and
> the STM32H7 driver behind it arrive with the ETH DMA descriptor work. Until
> then `net.h` / `net_config.h` include a header that does not exist and the
> `net` (NetIf) target does not compile. `lwip`, `packet`, `modbus` and
> `test/modbus` are independent of it and build on every preset.

```
Cable -> PHY -> MAC -> DMA -> RAM -> net.h -> pbuf -> IP -> TCP/UDP -> packet/ -> app
       |----- EoT::Ethernet<T> -----|  |-NetIf-|  |------- lwIP -------|  |-Tcp/UdpServer<T>-|
```

## Layout

```
net/
  CMakeLists.txt        lwip (vendored core + our port) and net (NetIf) libraries
  net.h / net_impl.h    NetIf<TEth>: link-state machine + lwIP netif glue
  net_config.h          NetConfig, LinkState, IpMode, StaticIpConfig
  lwip_port/
    lwipopts.h          lwIP compile-time options for this port (NO_SYS=1)
    arch/cc.h           compiler/platform hooks lwIP requires
    arch/sys_arch.*     sys_now() millisecond clock, backed by Utils::get_ms_ticks()
  packet/
    CMakeLists.txt      packet (INTERFACE) library
    packet_protocol.h   PacketProtocol<T> CRTP facade + concept
    tcp_server.h        TcpServer<TProtocol>: generic lwIP TCP server
    udp_server.h        UdpServer<TProtocol>: generic lwIP UDP server
    modbus/
      CMakeLists.txt    modbus library
      modbus.h          function codes, exception codes, Spec constants
      data.h            DataModel + Server (PDU in, PDU out; no network)
      mbap.h            MBAP header codec (PDU <-> ADU)
      protocol.h        Modbus::Protocol, the PacketProtocol for TCP/UDP
```

## The seams

The repo pattern is a generic CRTP facade with a concept stating the contract
and a `static_assert` in the constructor reporting a violation at the point of
instantiation. This folder has two of them:

| Facade                          | Below it                          | Above it                     |
|---------------------------------|-----------------------------------|------------------------------|
| `EoT::Ethernet<T>` (ethernet.h) | a platform driver (pending DMA merge) | `NetIf<TEth>`            |
| `Net::PacketProtocol<T>`        | a protocol (e.g. `Modbus::Protocol`) | `TcpServer<T>`, `UdpServer<T>` |

`PacketProtocol` is two calls. `frame_size(bytes, length)` says where one frame
ends in a byte buffer (`Complete` / `Incomplete` / `Invalid`), which is what lets
the TCP server reassemble a stream and the UDP server reject a truncated
datagram. `handle(frame, response)` turns one complete frame into one response;
returning 0 sends nothing. A protocol also exports `kMaxFrameSize` (sizes the
transport buffers) and `kDefaultPort`.

Lengths across this seam are `uint16_t`, not `size_t`: lwIP carries every
buffer length as `u16_t`, and `kMaxFrameSize` bounds every buffer a transport
owns, so a wider type would only be narrowed at the lwIP boundary.

Everything a BSP has to choose is a struct with defaults, in the same spirit as
`NetConfig`: `Modbus::ProtocolConfig` (unit id), `TcpServer<T>::Config` (port,
idle-poll tunables), `UdpServer<T>::Config` (port). The number of TCP slots is
a template parameter because it sizes a `std::array`.

The transports are protocol-agnostic and header-only. Adding a second protocol
means one class deriving from `PacketProtocol<>` and no transport changes.

## Wiring it up

Once a driver satisfying `Ethernet<T>` exists:

```cpp
SomeDriver eth_driver{...};               // derives from Ethernet<SomeDriver>
Net::NetIf<decltype(eth_driver)> netif{eth_driver};
netif.init(config);                       // lwip_init, netif_add, DHCP or static

Modbus::DataModel model{};                // spans onto app-owned arrays
model.holding_registers = holding_registers;

Modbus::Server   modbus{model};           // PDU dispatcher
Modbus::Protocol protocol{modbus};        // MBAP framing; {.unit_id = n} to filter
Net::TcpServer<Modbus::Protocol> tcp{protocol};     // <Protocol, 8> for 8 slots
Net::UdpServer<Modbus::Protocol> udp{protocol};
tcp.start();                              // Config{} -> port 502, 1 min idle drop
udp.start({.port = 1502});

while (true)
{
    netif.poll();                         // drains RX, runs lwIP timers, samples link
}
```

One `Server` and one `Protocol` shared by both transports means TCP and UDP
serve exactly the same register map by construction.

## What runs where

Everything runs in one context: the superloop, via `poll()`. lwIP callbacks
(`on_accept`, `on_recv`, ...) fire from inside `poll()`, so the servers' shared
`tx_buffer`s and the `DataModel` spans need no locking — and equally, none of
this may be touched from an ISR. A `WriteHook` on the data model runs inside
that same call, before the response is sent; keep it short.

## Host testing

`modbus/` (vocabulary, codec, dispatcher, protocol) and `packet_protocol.h`
never include lwIP and are unit tested on the host under `test/modbus`
(`native` preset, then `ctest` in `build/native`; see `test/README.md`).
`tcp_server.h` / `udp_server.h` and `net.h` do include lwIP; they build
natively too but need a driver behind them to do anything, so they get
exercised on the board once one exists.

## lwIP options worth knowing

`lwip_port/lwipopts.h` only lists deviations from `lwip/opt.h` defaults, with a
comment per choice. The ones that interact with this folder:

- `NO_SYS 1`, `LWIP_NETCONN 0`, `LWIP_SOCKET 0` — raw API only
- `PBUF_POOL_BUFSIZE 1524` — one linear buffer per received frame; `NetIf` does
  not build pbuf chains on RX
- `MEMP_NUM_TCP_PCB` — must be at least `TcpServer::kMaxConnections` per
  server instance plus listeners
- `CHECKSUM_GEN_* / CHECK_* 1` — all software until DMA descriptor offload exists
