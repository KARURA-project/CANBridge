#include <Arduino.h>
#include <CANBridge/Mcp2518.h>
using namespace canbridge;

Config config;
Bus bus;
bool ready = false;
Frame frame;
bool waitingToSend = false;

void setup() {
    Serial.begin(115200);
    config.bitrate = 1000000;
    config.spi = &SPI;
    config.csPin = SS;
    config.oscillatorHz = 40000000;
    SPI.begin(); // Configure SPI pins here first if your wiring uses other pins.

    const Result result = bus.begin(config);
    ready = result == Result::Ok;
    if (!ready) Serial.println(toString(result));
}

void loop() {
    if (!ready) return;
    if (!waitingToSend) {
        if (bus.receive(frame) != Result::Ok) return;
        waitingToSend = true;
    }
    const Result result = bus.send(frame);
    if (result == Result::Busy) return;
    if (result != Result::Ok) {
        Serial.println(toString(result));
        ready = false;
        return;
    }
    waitingToSend = false;
}
