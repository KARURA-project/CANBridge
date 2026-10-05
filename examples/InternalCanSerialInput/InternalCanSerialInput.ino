// 改行で送信。例: S 123#1122 / E 001ABCDE#0102 / S 123#R8。常時受信表示。
// 対応ボード・配線・試験手順: examples/README.md
#include <Arduino.h>
#include <CANBridge.h>
#include <CANBridge/EspCan.h>
#include <CANBridge/Examples.h> // サンプル用の入力・表示・試験処理。通常開発には必須ではありません。

// 1. 配線と合わせてここを変更してください。ボードのピン名を使用します。
constexpr std::uint32_t BITRATE = 1000000;
constexpr int TX_PIN = D0;
constexpr int RX_PIN = D1;

// 2. 宣言は全構成で共通です。
canbridge::Config config;
canbridge::Bus bus;
canbridge::examples::Runner<canbridge::Bus> runner(bus, canbridge::examples::Mode::SerialInput);

void setup() {
    Serial.begin(115200); // モニターを開いてからリセットすると起動時の表示を確認できます。
    config.bitrate = BITRATE;
    config.txPin = TX_PIN;
    config.rxPin = RX_PIN;

    // 初期設定後の操作は全構成で共通。診断モードではdiagnose()を実行します。
    runner.begin(config);
}

void loop() {
    // 送受信・入力・表示を少しずつ処理。通信モードではdelay()を使いません。
    runner.loop();
}
