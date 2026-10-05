# Dependency provenance

- ACAN2515: https://github.com/pierremolinaro/acan2515 — MIT, submodule pinned to 91896654358aa56c2b86ccd57e4d63659ca0d73b (2.1.5).
- ACAN2517FD (also supports MCP2518FD): https://github.com/pierremolinaro/acan2517FD — MIT, submodule pinned to 74e5b29fdbba16caeddac1d2535f49a24bb8b42c (2.1.16).
- ESP-IDF TWAI is provided by the installed Arduino-ESP32 framework, not bundled into this repository.

Dependency license texts are retained inside the submodules and distribution ZIP.

CANBridge adaptation: `src/dependencies/acan2515/ACAN2515.cpp` is derived from the pinned ACAN2515 source under its MIT license. ESP task/semaphore creation is removed and `poll()` executes synchronously. CANBridge uses no external INT pin. The upstream submodule remains unchanged.

For ACAN2517FD on ESP, CANBridge releases the upstream semaphore when destroying the backend, after upstream end() has stopped its task.

Bounded-diagnostic adaptations: ACAN2515 polling handles at most 64 events per call
and mode deadlines use wrap-safe elapsed time. Entering configuration mode aborts
pending transmissions so shutdown does not depend on an external ACK. `src/dependencies/acan2517FD/ACAN2517FD.cpp`
is now a derived copy of the pinned MIT source: its polling core is capped at 64
iterations and shutdown uses 100 bounded attempts with microsecond delays rather
than relying on millis while interrupts are disabled. Submodules remain unchanged.
