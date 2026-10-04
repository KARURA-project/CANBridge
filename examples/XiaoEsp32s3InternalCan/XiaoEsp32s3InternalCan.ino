// XiaoEsp32s3 / InternalCan: Classic CAN echo.
#include <Arduino.h>
#include <CANBridge.h>
using namespace canbridge;

// Match these settings to your wiring and module.
constexpr int kTxPin = D0;
constexpr int kRxPin = D1;
EspCan controller(kTxPin, kRxPin);
constexpr uint32_t kBitrate = 1000000;

bool ready = false;
Frame frame;
bool waitingToSend = false;

void setup() {
    Serial.begin(115200);
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
