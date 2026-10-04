# Validation

Hardware CAN traffic has not been tested. Build checks do not prove wiring,
transceiver compatibility, SPI signal integrity or loss-free operation.

- Host frame validation: passed (standard/extended ID and payload bounds).
- XIAO ESP32S3, Arduino-ESP32 2.0.17: integrated library build passed.
- ESP32-C5/new TWAI, Pico, Pico 2 and XIAO RP2350: build checks in progress.

The checked-in PlatformIO matrix includes both SPI controllers on each board
and internal CAN on ESP. It pins the RP platform commit and ESP tool versions.
