// RaspberryPiPico / Mcp2515: Classic CAN echo.
#include <Arduino.h>
#include <CANBridge.h>
using namespace canbridge;

// Match these settings to your wiring and module.
constexpr uint8_t kChipSelect = SS;
constexpr uint32_t kOscillatorHz = 16000000;
Mcp2515 controller(SPI, kChipSelect, kOscillatorHz);
constexpr uint32_t kBitrate = 1000000;

bool ready = false;
Frame frame;
bool waitingToSend = false;

void setup() {
    Serial.begin(115200);
    SPI.begin(); // Uses this board core's default SPI pins.
    Config config;
    config.bitrate = kBitrate;
    ready = controller.begin(config) == Result::Ok;
    if (!ready) Serial.println("CAN initialization failed");
}

void loop() {
    if (!ready) return;

    if (!waitingToSend) {
        if (controller.receive(frame) != Result::Ok) return;
        waitingToSend = true;
    }

    const Result result = controller.send(frame);
    if (result == Result::Busy) return; // Keep the frame and retry next loop.
    if (result != Result::Ok) {
        Serial.println("CAN send failed");
        ready = false;
        return;
    }
    waitingToSend = false;
}
