# Design boundary

Application code owns timing, routing and actuator safety policy.
`Controller` owns CAN transport and reports transport results only.
No backend includes RoboMaster or any motor protocol header.

## Interchangeability

Swap the controller construction, SPI initialization and wiring configuration.
The `begin / send / receive / pollHealth / end` calls and `Frame` stay identical.
This does not mean identical FIFO depth, latency, oscillator choices, driver
allocation behavior or complete fault observability on every backend.
Unsupported configurations and invalid frame formats are explicitly rejected.

Classic CAN is the common baseline: standard/extended identifiers, remote/data
frames and 0–8 byte payloads. CAN FD requires a later explicit frame/capability
extension; FD payloads must never be silently truncated into Classic frames.

## Lifetime and concurrency

An instance is not copyable and must outlive the active driver. Call end before
reinitializing or destroying it. All public calls run on one application task.
ESP new-SDK RX is queued from callbacks; its TX frame remains in instance-owned
storage until the SDK reports completion. A busy return does not accept ownership
of the caller frame. External drivers use upstream polling, avoiding global ISR
routing and preserving independent controller instances.

## Dependency boundary

MCP2515 / MCP2518FD underlying drivers are read-only pinned submodules.
Tiny build forwarding sources compile their original source files as part of
KaruraCAN. Neither Arduino nor PlatformIO users need separately installed ACAN
libraries. ESP-IDF driver code belongs to the Arduino board framework and is
not duplicated as a second independent SDK checkout.

## Health limitations

Health is a snapshot, not an event queue. Raw fields are backend diagnostics:
MCP2515 EFLG, MCP2518FD TREC, ESP cumulative bus-error count.
A nonzero receive-loss report requests application-level recovery. Upstream
peak counts and overflow indicators are inspected where available, but lack of
an indicator is not proof of continuous reception. Bus-off/error-passive fields
are controller current state; silicon recovery behavior may differ. KaruraCAN
issues no automatic software recovery and never resumes application commands.

## Distribution

Development checkouts preserve gitlinks. Arduino releases expand the pinned
submodules into a single installable ZIP while preserving licenses and source
paths. PlatformIO Git dependencies recursively initialize submodules; ZIP use
is also supported. Installing a board core is always a prerequisite.
