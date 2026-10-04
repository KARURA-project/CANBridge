# Examples

Each example selects exactly one board/controller combination without preprocessor
branches or commented-out alternative implementations. Open its `.ino`, select the
board in Arduino IDE, and adjust the visible wiring/bitrate settings.

| Board | CAN controller | Sketch |
| --- | --- | --- |
| XiaoEsp32s3 | InternalCan | [XiaoEsp32s3InternalCan](XiaoEsp32s3InternalCan/XiaoEsp32s3InternalCan.ino) |
| XiaoEsp32s3 | Mcp2515 | [XiaoEsp32s3Mcp2515](XiaoEsp32s3Mcp2515/XiaoEsp32s3Mcp2515.ino) |
| XiaoEsp32s3 | Mcp2518 | [XiaoEsp32s3Mcp2518](XiaoEsp32s3Mcp2518/XiaoEsp32s3Mcp2518.ino) |
| XiaoEsp32c5 | InternalCan | [XiaoEsp32c5InternalCan](XiaoEsp32c5InternalCan/XiaoEsp32c5InternalCan.ino) |
| XiaoEsp32c5 | Mcp2515 | [XiaoEsp32c5Mcp2515](XiaoEsp32c5Mcp2515/XiaoEsp32c5Mcp2515.ino) |
| XiaoEsp32c5 | Mcp2518 | [XiaoEsp32c5Mcp2518](XiaoEsp32c5Mcp2518/XiaoEsp32c5Mcp2518.ino) |
| XiaoRp2350 | Mcp2515 | [XiaoRp2350Mcp2515](XiaoRp2350Mcp2515/XiaoRp2350Mcp2515.ino) |
| XiaoRp2350 | Mcp2518 | [XiaoRp2350Mcp2518](XiaoRp2350Mcp2518/XiaoRp2350Mcp2518.ino) |
| RaspberryPiPico | Mcp2515 | [RaspberryPiPicoMcp2515](RaspberryPiPicoMcp2515/RaspberryPiPicoMcp2515.ino) |
| RaspberryPiPico | Mcp2518 | [RaspberryPiPicoMcp2518](RaspberryPiPicoMcp2518/RaspberryPiPicoMcp2518.ino) |
| RaspberryPiPico2 | Mcp2515 | [RaspberryPiPico2Mcp2515](RaspberryPiPico2Mcp2515/RaspberryPiPico2Mcp2515.ino) |
| RaspberryPiPico2 | Mcp2518 | [RaspberryPiPico2Mcp2518](RaspberryPiPico2Mcp2518/RaspberryPiPico2Mcp2518.ino) |

## Wiring settings

- Internal CAN: TX is D0 and RX is D1. Connect a compatible external transceiver.
- SPI controllers: use the board core's default SPI pins and SS for chip select.
  Change `config.csPin` if needed. The external INT pin is not used (polling).
- MCP2515 uses a 16 MHz crystal setting; MCP2518FD uses 40 MHz. Change
  `config.oscillatorHz` to the actual module crystal frequency.
- All examples use 1 Mbit/s Classic CAN; MCP2518FD examples do not enable CAN FD.
- Supply the correct voltage, common ground and CAN termination for your hardware.

## Minimal flow

Initialize, receive one frame, then echo it. While transmission returns Busy,
keep that frame and retry without replacing it with another received frame.
Initialization/send failures print a message and stop the example. Receive results
other than Ok simply return to the next loop; these examples do not diagnose
receive faults or perform health monitoring/recovery. For those, use
`pollHealth(Health&)` as described in the library README.

These are short communication examples, not actuator safety templates. An echo
requires another CAN node and can loop indefinitely if both nodes echo traffic.
Serial baud is 115200. No physical hardware execution has been verified yet.
For PlatformIO, place the chosen sketch contents in `src/main.cpp` and specify
CANBridge as the library dependency; the code uses explicit setup/loop functions
and needs no generated function declarations.

Each selected controller header supplies `canbridge::Config` and `canbridge::Bus`.
Required settings are written with `config.` in setup; none default to a working
hardware configuration. Initialization errors print `toString(result)`, which
identifies missing settings. The loop is identical in all twelve examples.
