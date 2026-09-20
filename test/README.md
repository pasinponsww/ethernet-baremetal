# `test/` — host unit tests

GoogleTest, built and run on the host only (`native` preset). Nothing in here
is compiled for an MCU target. One subdirectory per unit under test, mirroring
the source tree it covers.

```
test/
  CMakeLists.txt        add_subdirectory per unit
  modbus/               common/core/net/packet/modbus
    mbap_test.cc        MBAP header codec
    data_test.cc        DataModel + Server (PDU dispatcher)
    protocol_test.cc    Modbus::Protocol against the PacketProtocol contract
  packet/               common/core/net/packet
    transport_test.cc   explicit instantiation of TcpServer<T> / UdpServer<T>
```

## Running

```powershell
./make.ps1 -t native          # or: cmake --preset native && cmake --build build/native
pushd build/native
ctest --output-on-failure
popd
```

A single binary can be run directly for gtest's own filtering and output:

```powershell
./build/native/test/modbus/data_test.exe --gtest_filter='DataTest.Write*'
```

## Adding a test

1. Create `test/<unit>/<name>_test.cc` containing `TEST` / `TEST_F` cases.
2. List it in `test/<unit>/CMakeLists.txt`:

   ```cmake
   add_tests(<library-to-link>
       <name>_test
   )
   ```

   `add_tests` (in `cmake/build-util.cmake`) builds each name as `<name>.cc`
   against `GTest::gtest_main` and the given library, and registers it with
   ctest. No `main()` needed. Several libraries: `add_tests("modbus;lwip" ...)`.

3. New unit? Add `add_subdirectory(<unit>)` to `test/CMakeLists.txt`.

What can be tested here is whatever links without hardware behind it.
`common/core/net/packet/modbus` never includes lwIP. `TcpServer<T>` /
`UdpServer<T>` do, but lwIP itself compiles on the host, so they can be
constructed and instantiated here; only `start()` and the callbacks need a
live stack and are exercised on the board instead.

## Modbus tests

The Modbus layer is three byte-in/byte-out units stacked on each other, and the
tests are split the same way so a failure points at one layer.

### `mbap_test` — `modbus/mbap.h`

The 7-byte MBAP header codec. The thing worth being paranoid about is the
length field: it counts everything *after* itself (unit id + PDU), so a 5-byte
PDU gives `length == 6`. Counting from the wrong base is the classic Modbus TCP
bug, and `EncodeRecomputesLengthFromPduSize` / `RoundTrip` pin it down.

| Test | Checks |
|---|---|
| `DecodesEveryField` | transaction id, protocol id, length, unit id land in the right members |
| `PduSizeExcludesUnitId` | `pdu_size() == length - 1`, and 0 for a degenerate length |
| `ShortBufferIsIncomplete` | < 7 bytes is `Incomplete`, not an error |
| `NonZeroProtocolIdIsRejected` | `BadProtocolId` |
| `ZeroLengthIsRejected`, `LengthOverMaxPduIsRejected` | `BadLength` at both bounds |
| `EncodeRecomputesLengthFromPduSize` | the caller's `length` is ignored; the output is `pdu_size + 1` |
| `EncodeRefusesSmallBufferAndBadPduSize` | returns false and writes nothing |
| `RoundTrip` | encode then decode yields the same ids and PDU size |

### `data_test` — `modbus/data.h`

The PDU dispatcher over a fixture with 16 coils, 16 discrete inputs, 8 holding
registers and 8 input registers, plus a `WriteHook` that records every
notification. Every function code has a happy-path test asserting the exact
response bytes, and every validation rule has a test asserting the exception
code it produces:

- **Bounds** — `address + quantity > table size` is `IllegalDataAddress`; an
  empty table is always `IllegalDataAddress`.
- **Quantities** — zero, or over the spec limit (`Spec::kMaxRead*` /
  `kMaxWrite*`), is `IllegalDataValue`.
- **Short PDUs** — a request too short to hold its own fields is
  `IllegalDataValue`, never an out-of-bounds read. An *empty* PDU is the one
  silent case (no function code to build an exception around) and is counted
  in `Stats::malformed`.
- **FC 5 magic values** — only `0xFF00` / `0x0000` are accepted.
- **FC 15/16 consistency** — quantity, byte count and actual payload length
  must all agree.
- **Bit packing** — coils go on the wire LSB-first within each byte
  (`ReadCoilsPacksLsbFirst`, `WriteMultipleCoilsUnpacksLsbFirst`).
- **Write hook** — fires once per successful write with the right function,
  address and quantity, and not at all on a rejected one.
- **Response buffer too small** — `ServerDeviceFailure`, not a truncated write.

### `protocol_test` — `modbus/protocol.h`

`Modbus::Protocol` as seen by a transport, i.e. the two `PacketProtocol` calls:

- `frame_size` returns `Complete` with the ADU length for a full ADU, and only
  the *first* ADU when several are pipelined in one buffer (what lets
  `TcpServer<T>` drain a segment carrying more than one request). A partial
  header, or a header whose body has not arrived, is `Incomplete`; a foreign
  protocol id is `Invalid`.
- `handle` unwraps the ADU, dispatches the PDU, and wraps the response with
  the transaction and unit id echoed and the length recomputed. Exception
  responses get the same wrapping.
- Unit-id filtering: with the default `ProtocolConfig` it answers any unit;
  with `ProtocolConfig{.unit_id = n}` it drops everything else silently and
  counts it in `ProtocolStats::filtered`.

### `transport_test` — `packet/tcp_server.h`, `packet/udp_server.h`

The transports are templates, so nothing compiles their bodies until they are
instantiated. This file does `template class TcpServer<Modbus::Protocol>;`
(and the UDP one), which forces every member to compile against the real
`PacketProtocol` contract; a signature drift fails here instead of in an
application build. It also checks a freshly constructed server is idle,
`stop()` on it is a no-op, and that the `kMaxConnections` template parameter
scales the object.

## Not covered (yet)

- `TcpServer<T>` / `UdpServer<T>` behaviour: stream reassembly, slot
  exhaustion, idle timeout. Would need a fake `PacketProtocol` and either a
  lwIP loopback netif or a mock of the handful of `tcp_*` / `udp_*` calls used.
- `NetIf<TEth>`: link state machine. Would need a fake `Ethernet<T>`.
