# Validation

See [構成別の検証状況・実機記録](VALIDATION.md) for current hardware coverage.

## Host checks

Run `python3 tests/host/run.py` with a C++ compiler installed. It compiles with
C++11, warnings, AddressSanitizer and UndefinedBehaviorSanitizer and tests:

- Standard/extended/data/RTR parsing and formatting, invalid IDs and line data.
- Successful loopback, mismatch, timeout including millis wraparound.
- Access/operation failure leaves later tests unexecuted.
- Cleanup failure blocks restart; successful end retry releases the backend.
- Serial backpressure does not prevent receive processing.
- Busy TX preserves its frame, continues RX and rejects a second complete command.

These tests use fake backends and do not validate physical controllers.

## Embedded builds

Compile each consolidated sketch in Arduino IDE for each supported board, or use Arduino CLI:

```sh
arduino-cli compile --fqbn esp32:esp32:XIAO_ESP32C5 --library "$PWD" examples/InternalCanDiagnostic
```

Use XIAO_ESP32S3 for S3 and the matching Arduino-Pico board for RP targets.
When using PlatformIO, copy a sketch to src/main.cpp and use this repository as
a local symlink library dependency. All examples declare setup/loop explicitly.

The implementation is checked against Arduino-Pico 5.5.1, Arduino-ESP32 2.0.17
(legacy TWAI on S3), and Arduino-ESP32 3.3.10 (new TWAI on S3/C5).
Internal diagnostics passed on S3, both C5 TWAI controllers, and Pico 2 + MCP2515 (8 MHz, 500 kbps).
Other SPI configurations and external communication tests remain outstanding.
Use [C5TwoControllers](hardware/C5TwoControllers/README.md) to check both C5 controllers.

## Hardware acceptance checklist

1. Run Diagnostic with ESP TX unconnected, or with the powered SPI module attached.
   Confirm all four stages Passed and frames=4. Reset and repeat to check stability.
2. Call diagnose(), then begin()/end(), then diagnose() again in the same process.
3. Remove SPI wiring/power: diagnostic must fail and return, not freeze.
   Restore wiring and retry end() before restarting if cleanup failed.
4. Connect two nodes through transceivers at the same bitrate and run Periodic
   against Echo. Verify sequence payloads are returned in RX lines.
5. Run SerialInput against Echo; test zero/eight bytes, extended IDs and RTR DLC.
6. Disconnect the peer; TX accepted must not be interpreted as delivery.
7. Stop reading serial and send input/traffic; CAN processing should continue and
   output overload should be reported after reading resumes.

Loopback does not certify normal RX wiring, the transceiver, CAN termination or
actual bitrate/crystal accuracy. Do not use two echo nodes or an actuator bus.
