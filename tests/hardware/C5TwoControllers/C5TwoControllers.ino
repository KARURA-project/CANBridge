// C5の2コントローラーを順に自己診断。D0/D1/D2は未接続にしてください。
#include <Arduino.h>
#include <CANBridge.h>
#include <CANBridge/EspCan.h>
#if !defined(CONFIG_IDF_TARGET_ESP32C5) || ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5,5,0)
#error "Select XIAO ESP32C5 with ESP-IDF 5.5+"
#endif
#include <esp_arduino_version.h>

canbridge::Config config;
canbridge::Bus bus;

void printStep(const char *name, const canbridge::DiagnosticStep &step) {
    Serial.printf("DIAG %s: %s; %s; raw=0x%08lX\n", name,
        canbridge::toString(step.status), canbridge::toString(step.reason), (unsigned long)step.raw);
}
void runDiagnostic(const char *name) {
    Serial.printf("TEST %s\n", name);
    canbridge::DiagnosticReport report;
    const auto result = bus.diagnose(config, report);
    printStep("access", report.controllerAccess);
    printStep("operation", report.controllerOperation);
    printStep("loopback", report.internalLoopback);
    printStep("cleanup", report.cleanup);
    Serial.printf("DIAG result: %s; frames=%lu\n", canbridge::toString(result), (unsigned long)report.framesChecked);
}
void setup() {
    Serial.begin(115200);
    delay(1000); // 起動時だけ。モニターを開いてからリセットしてください。
    Serial.printf("Arduino-ESP32 %s; ESP-IDF %s\n", ESP_ARDUINO_VERSION_STR, esp_get_idf_version());
    config.bitrate = 1000000;
    config.txPin = D0;
    config.rxPin = D1;

    // このスケッチ以外がTWAIを確保していない状態で実行します。
    // SDKは空きスロットを0から順に割り当てます。
    runDiagnostic("TWAI0 (first allocation)");
    // 診断の終了失敗時は次へ進まず、その結果を調べてください。
    const auto stopped = bus.end();
    if (stopped != canbridge::Result::NotStarted) {
        Serial.println("STOP: first diagnostic did not release its controller; reset and inspect the log");
        return;
    }

    twai_onchip_node_config_t reserveConfig{};
    reserveConfig.io_cfg.tx = static_cast<gpio_num_t>(D2);
    reserveConfig.io_cfg.rx = static_cast<gpio_num_t>(D2);
    reserveConfig.io_cfg.quanta_clk_out = GPIO_NUM_NC;
    reserveConfig.io_cfg.bus_off_indicator = GPIO_NUM_NC;
    reserveConfig.bit_timing.bitrate = config.bitrate;
    reserveConfig.tx_queue_depth = 1;
    reserveConfig.flags.enable_self_test = true;
    reserveConfig.flags.enable_loopback = true;
    twai_node_handle_t reserved = nullptr;
    const auto reserveResult = twai_new_node_onchip(&reserveConfig, &reserved);
    Serial.printf("RESERVE TWAI0: 0x%08lX (not enabled, no transmission)\n", (unsigned long)reserveResult);
    if (reserveResult != ESP_OK) return;
    // TWAI0が確保されているので、CANBridgeの次の割り当てはTWAI1になります。
    runDiagnostic("TWAI1 (TWAI0 reserved)");
    const auto releaseResult = twai_node_delete(reserved);
    Serial.printf("RELEASE TWAI0: 0x%08lX\n", (unsigned long)releaseResult);
    Serial.println("DONE");
}
void loop() {}
