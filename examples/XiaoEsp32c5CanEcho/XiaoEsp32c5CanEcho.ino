// Board: XiaoEsp32c5. Set pins/crystal below to match your wiring.
#include <CANBridge.h>
using namespace canbridge;
#ifdef ARDUINO_ARCH_ESP32
EspCan controller(1, 2); // Change to the actual TX/RX GPIOs on your board.
#else
Mcp2515 controller(SPI, SS, 16000000); // Actual CS and crystal frequency required.
// Mcp2518 controller(SPI, SS, 40000000);
#endif
bool failed = false;
Frame pending;
bool hasPending = false;
void setup() {
    Serial.begin(115200);
#ifndef ARDUINO_ARCH_ESP32
    SPI.begin(); // Configure SPI pins for your board before this call.
#endif
    Config config;
    failed = controller.begin(config) != Result::Ok;
}
void loop() {
    if (failed) return;
    Health health;
    if (controller.pollHealth(health) != Result::Ok || health.busOff ||
        health.errorPassive || health.receiveLoss) { failed=true; return; }
    if (!hasPending) {
        Result r=controller.receive(pending);
        if(r==Result::Empty) return;
        if(r!=Result::Ok) {failed=true;return;}
        hasPending=true;
    }
    Result sent=controller.send(pending);
    if(sent==Result::Ok)hasPending=false;
    else if(sent!=Result::Busy)failed=true;
}
