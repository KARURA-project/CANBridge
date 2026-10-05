# Build and regression results — 2026-10-05

Current per-configuration hardware status: [VALIDATION.md](VALIDATION.md).
Host regression tests were rerun successfully on 2026-10-06.

Samples are consolidated into 12 sketches (3 controllers × 4 modes), compiled
for all 12 supported configurations (48 combinations). SPI initialization uses
explicit pins with Arduino-Pico/ESP32 branches.

Working branch: feature/unified-diagnostics. Hardware checks were performed before committing;
see each hardware record for source identification.

| Target | Core | Sketches | Result |
| --- | --- | --- | --- |
| XIAO RP2350, Pico, Pico 2 × MCP2515/MCP2518 | Arduino-Pico 5.5.1 | Echo/Periodic/SerialInput/Diagnostic, 24 builds | Passed |
| XIAO ESP32S3 × TWAI/MCP2515/MCP2518 | Arduino-ESP32 2.0.17 | All four, 12 builds | Passed |
| XIAO ESP32C5 × TWAI/MCP2515/MCP2518 | Arduino-ESP32 3.3.10 | All four, 12 builds | Passed |
| XIAO ESP32S3 × TWAI/MCP2515/MCP2518 | Arduino-ESP32 3.3.10 | All four, 12 additional builds | Passed |

After the final MCP2515 shutdown-abort change, all nine RP/legacy-S3 diagnostic
builds and the C5/new-S3 MCP2515 diagnostic builds were rerun successfully.

Host C++11 tests with AddressSanitizer/UndefinedBehaviorSanitizer: passed.
Whitespace/error-marker check: passed. Pinned submodules: unchanged.

Arduino CLI used the installed core and an explicit library path. Its bundled
x86 ctags was bypassed with tools.ctags.cmd.path=/usr/bin/true; these sketches
need no generated function declarations. PlatformIO used the same working
repository through a symlink library dependency.

Hardware internal diagnostics passed on S3, both C5 controllers, and Pico 2 + MCP2515.
Other configurations, SPI disconnection behavior, cleanup/restart under actual faults,
and external CAN traffic remain unverified.
Use the hardware checklist in tests/README.md before treating these as validated
hardware diagnostics.
