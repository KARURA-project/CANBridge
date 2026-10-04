# Samples

Each board has one Classic CAN echo sample. Open its `.ino` in Arduino IDE.
Select your board core, set the controller/pins/crystal frequency, then build.
The receive/send/health loop is identical across boards. A received frame is
held while `send` returns Busy; it is never silently discarded.

- ESP samples default to built-in CAN (TX GPIO1 / RX GPIO2 are editable wiring values).
- RP samples default to MCP2515 using SPI's default pins and SS chip select,
  with a 16 MHz crystal. Select Mcp2518 for that module and its actual crystal.
- ESP can also select Mcp2515/Mcp2518; call SPI.begin() in setup when doing so.
- Echo needs a second CAN node and can create a feedback loop if that node also
  echoes. These are communication examples, not motor tests.
- A 3.3 V-compatible transceiver, common ground, power and termination are required.
- No hardware execution has been verified yet.
