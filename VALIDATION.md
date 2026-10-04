# Validation

Hardware CAN traffic has not been tested. Build checks do not prove wiring,
transceiver compatibility, SPI signal integrity or loss-free operation.

## Passed

GitHub Actions run 37195523026 (commit 808ee9d):
https://github.com/KARURA-project/CANBridge/actions/runs/37195523026

- Host frame validation: standard/extended ID and payload bounds.
- XIAO ESP32S3: internal TWAI, MCP2515, MCP2518FD.
- XIAO ESP32C5: internal TWAI, MCP2515, MCP2518FD.
- Raspberry Pi Pico: MCP2515, MCP2518FD.
- Raspberry Pi Pico 2: MCP2515, MCP2518FD.
- XIAO RP2350: MCP2515, MCP2518FD.
- Distribution ZIP generation with recursively checked-out dependencies.
- Local ZIP inspection: relative source/header dependencies and both upstream licenses included.

The Linux build matrix pins ESP32S3 to Espressif32 6.13.0, C5 to pioarduino
55.03.37 (Arduino 3.3.7 / IDF 5.5), and the RP platform to commit
79ce473e1a8010ed8096388690222fd2c94858a6 (Arduino-Pico).

Local macOS RP build initially failed because the library forced C++11 on a
newer Arduino core. Removing that override fixed the Linux target matrix.
No assertion of macOS completion or physical hardware testing is made.

## Required hardware verification

For each selected controller/board wiring: reception and acknowledged
transmission with a second node, queue saturation, extended/remote frames,
bus-off/error-passive reporting, restart, and sustained traffic under expected
application load. Validate actual module crystal frequency and transceiver voltage.
