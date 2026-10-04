#include <KaruraCAN.h>
using namespace karura::can;
#if defined(TEST_MCP2515)
Mcp2515 controller(SPI, 17, 16000000);
#elif defined(TEST_MCP2518)
Mcp2518 controller(SPI, 17, 40000000);
#elif defined(ARDUINO_ARCH_ESP32)
EspCan controller(1, 2); // Change to the actual TX/RX GPIOs on your board.
#else
Mcp2515 controller(SPI, 17, 16000000); // Actual CS and crystal frequency required.
// Mcp2518 controller(SPI, 17, 40000000);
#endif
bool failed = false;
Frame pending;
bool hasPending = false;
void setup() {
    Serial.begin(115200);
#if !defined(ARDUINO_ARCH_ESP32) || defined(TEST_MCP2515) || defined(TEST_MCP2518)
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
