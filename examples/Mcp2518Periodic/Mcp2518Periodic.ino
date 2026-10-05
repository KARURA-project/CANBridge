// 周期送信しながら常時受信表示。相手をEchoにすると往復を確認できます。
// 対応ボード・配線・試験手順: examples/README.md
#include <Arduino.h>
#include <CANBridge.h>
#include <CANBridge/Mcp2518.h>
#include <CANBridge/Examples.h> // サンプル用の入力・表示・試験処理。通常開発には必須ではありません。

// 1. 配線と合わせてここを変更してください。既定値はボード標準のSPIピンです。
constexpr std::uint32_t BITRATE = 1000000;
constexpr int SCK_PIN = SCK;
constexpr int MISO_PIN = MISO;
constexpr int MOSI_PIN = MOSI;
constexpr int CS_PIN = SS;
constexpr std::uint32_t OSCILLATOR_HZ = 40000000; // モジュールの実際の水晶周波数

// 2. 宣言は全構成で共通です。
canbridge::Config config;
canbridge::Bus bus;
canbridge::examples::Runner<canbridge::Bus> runner(bus, canbridge::examples::Mode::Periodic);

void setup() {
    Serial.begin(115200); // モニターを開いてからリセットすると起動時の表示を確認できます。
    config.bitrate = BITRATE;
    config.spi = &SPI;
    config.csPin = CS_PIN;
    config.oscillatorHz = OSCILLATOR_HZ;

    // 3. SPIピンの指定方法だけがRP/ESPで異なります。
#if defined(ARDUINO_ARCH_RP2040)
    // Arduino-PicoではRP2350もこの分岐です。SPIに対応したピンを選びます。
    const bool sckOk = SPI.setSCK(SCK_PIN);
    const bool misoOk = SPI.setMISO(MISO_PIN);
    const bool mosiOk = SPI.setMOSI(MOSI_PIN);
    if (!sckOk || !misoOk || !mosiOk) {
        Serial.println("Invalid SPI pins; check the selected SPI bus");
        return;
    }
    SPI.begin(); // CSはCANドライバーが制御するためsetCS()は不要です。
#elif defined(ARDUINO_ARCH_ESP32)
    SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN); // 引数の順序に注意
#else
#error "Use Arduino-Pico or Arduino-ESP32"
#endif

    runner.periodMs = 1000; // 送信周期(ms)
    runner.periodicId = 0x123; // 標準CAN ID、データは4バイトの連番
    // 初期設定後の操作は全構成で共通。診断モードではdiagnose()を実行します。
    runner.begin(config);
}

void loop() {
    // 送受信・入力・表示を少しずつ処理。通信モードではdelay()を使いません。
    runner.loop();
}
