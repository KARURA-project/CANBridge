# KaruraCAN

モーターに依存しない、Arduino向けの統合 **Classic CAN** ライブラリです。
ESP内蔵TWAI・MCP2515・MCP2518FDで同じフレームと送受信APIを使います。
CAN FDのデータフレームは初版の対象外です。MCP2518FDとC5もClassic CANで使用します。

## 対象

| ボード | 内蔵CAN | 外付け |
| --- | --- | --- |
| XIAO ESP32S3 | EspCan | Mcp2515 / Mcp2518 |
| XIAO ESP32C5 | EspCan（ESP-IDF 5.5以降） | Mcp2515 / Mcp2518 |
| XIAO RP2350 | 使用しない | Mcp2515 / Mcp2518 |
| Raspberry Pi Pico | 使用しない | Mcp2515 / Mcp2518 |
| Raspberry Pi Pico 2 | 使用しない | Mcp2515 / Mcp2518 |

RP系はArduino-Pico（Earle Philhower）を対象とします。
対応実装と実機検証は別です。現時点で実機での通信確認はしていません。
ビルド結果は [VALIDATION.md](VALIDATION.md) に記録します。

## インストール

開発用は必ずサブモジュールを含めて取得してください。

```sh
git clone --recurse-submodules https://github.com/KARURA-project/KaruraCAN.git
```

通常のGitHub「Download ZIP」にはサブモジュールの中身が含まれません。
Arduino IDEは、リリース添付の `KaruraCAN.zip` を「ZIP形式のライブラリをインストール」で読み込んでください。
これには固定された依存ソース・ライセンスを含み、追加ライブラリのインストールは不要です。
開発チェックアウトからは `python3 scripts/package.py` で同じZIPを作成できます。
PlatformIO 6.1.18はGit依存を再帰取得するため、認証できる環境なら次の指定1つで利用できます。

```ini
lib_deps = https://github.com/KARURA-project/KaruraCAN.git
```

再現性を保つ場合は末尾に `#タグ名` または `#コミットSHA` を指定してください。
PlatformIOでも自己完結したZIPを `lib_deps` に指定できます。
ローカルの再帰取得済みリポジトリは `KaruraCAN=symlink:///absolute/path/KaruraCAN` で使えます。
ボードのArduinoコア・SPI実装は開発環境側の依存です。

## 共通API

```cpp
#include <KaruraCAN.h>
using namespace karura::can;
EspCan controller(txPin, rxPin);             // ESP内蔵CAN
// Mcp2515 controller(SPI, csPin, 16000000);  // モジュールの実際の発振器周波数
// Mcp2518 controller(SPI, csPin, 40000000);
Config config;
config.bitrate = 1000000;
controller.begin(config);
Frame frame;
controller.receive(frame);
controller.send(frame);
Health health;
controller.pollHealth(health);
```

`Frame` は `id / length / data[8] / extended / remote`。
`Controller&` を使えばアプリ側の通信コードは実装に依存しません。
生成時のピン・SPI・発振器設定だけを変えます。
SPIのピン設定と `SPI.begin()` はアプリ側で行います。外付け実装はポーリング方式です。
外付けのINTピンは使用せず、割り込み関数も不要です。
`receive()` と `pollHealth()` は頻繁に呼び出してください。
MCP2518FDの発振器は4/20/40 MHzを受け付けます。PLL設定は初版では公開していません。
ESP旧APIは125/250/500/1000 kbit/s、新APIと外付けは基盤のタイミング計算に従います。
非対応設定は成功扱いにしません。

### 結果と所有権

- `receive(Empty)` はデータなし。出力引数を変更しません。
- `send(Ok)` は送信受付。ACK・物理送信完了の保証ではありません。
- `send(Busy)` は未受付。同じフレームを保持して後で再試行します。
- `InvalidFrame` はIDまたは長さが不正。
- `pollHealth(Ok)` は状態取得成功。異常なしを意味せず、各フィールドを確認します。
- モーター、時計、タイマー、目標値、送信周期、安全停止方針を所有しません。
- mainでRoboMaster等とのフレーム変換・異常通知を行います。
- 1つのインスタンスを複数タスク・ISRから同時に操作しません。
- インスタンスは稼働中に破棄しません。終了時は `end()` を呼びます。
- ESP旧SDKでは内蔵CANインスタンスは1つです。初版は複数内蔵CANノードを対象にしません。

### 異常と復旧

Bus-off / Error-passiveと、観測可能なハードウェア・ソフトウェア受信欠落を報告します。
`receiveLoss` は観測した欠落を示します。ただし基盤やSDKが公開しない欠落もあるため、
`lossDetectionComplete` はfalseです。「falseだから絶対に欠落していない」とは解釈しません。
各値は現在状態・累積またはラッチで、読み出しても消去しません。イベント回数ではありません。
Bus-off等からの自動復旧や、古いアプリ指令の自動再開は行いません。
アプリが停止し、原因を解消して `end()/begin()` で再開してください。
CANトランシーバーの電圧・終端・EN/STB固定配線を確認してください。
トランシーバー制御は初版に含めません。

## 開発と配布

ACAN2515 / ACAN2517FDをコミット固定のサブモジュールで読み込みます。
`src`の小さな転送ファイルにより、ライブラリを1つ読み込むだけで依存がビルドされます。
ESPはArduinoコア同梱の公式ESP-IDF TWAIドライバーを直接使用します。
受信欠落の観測や送信バッファ寿命を保つため、別のESPラッパーは使用しません。
依存更新時はサブモジュールのコミット、ビルド、ライセンスをまとめて確認します。

```sh
c++ -std=c++11 -Isrc tests/frame.cpp -o /tmp/can-frame-test
/tmp/can-frame-test
pio run -d tests/embedded
python3 scripts/package.py
```

仕様資料: [ESP TWAI](https://docs.espressif.com/projects/esp-idf/en/v5.5.3/esp32c5/api-reference/peripherals/twai.html)、
[ACAN2515](https://github.com/pierremolinaro/acan2515)、
[ACAN2517FD](https://github.com/pierremolinaro/acan2517FD)。
