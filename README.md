# CANBridge

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
対応実装・ビルド確認と実機検証は別です。全12構成で4種類のサンプル（計48組み合わせ）のビルドを確認しています。
RP系はArduino-Pico 5.5.1、ESP32S3の旧TWAI APIはArduino-ESP32 2.0.17で確認しました。
ESP32S3とESP32C5の新TWAI APIはArduino-ESP32 3.3.10で完全ビルドを確認しました。
ESP32C5内蔵CANはTWAI0/TWAI1両基の単体診断が実機で各1回成功しました（Arduino-ESP32 3.3.10 / ESP-IDF v5.5.4）。
ESP32S3内蔵CANの単体診断も実機で1回成功しました（実行コア版は未確認）。
Pico 2 + MCP2515（8 MHz、500 kbps、3.3 V給電）の単体診断も実機で1回成功しました。
外部通信とその他のSPI構成の実機検証は未実施です。
構成別の最新状況と追加検証の記録は[検証状況](tests/VALIDATION.md)にまとめています。

## インストール

開発用は必ずサブモジュールを含めて取得してください。

```sh
git clone --recurse-submodules https://github.com/KARURA-project/CANBridge.git
```

通常のGitHub「Download ZIP」にはサブモジュールの中身が含まれません。
Arduino IDEは、リリース添付の `CANBridge.zip` を「ZIP形式のライブラリをインストール」で読み込んでください。
これには固定された依存ソース・ライセンスを含み、追加ライブラリのインストールは不要です。

PlatformIO 6.1.18はGit依存を再帰取得するため、認証できる環境なら次の指定1つで利用できます。

```ini
lib_deps = https://github.com/KARURA-project/CANBridge.git
```

再現性を保つ場合は末尾に `#タグ名` または `#コミットSHA` を指定してください。
PlatformIOでも自己完結したZIPを `lib_deps` に指定できます。
ローカルの再帰取得済みリポジトリは `CANBridge=symlink:///absolute/path/CANBridge` で使えます。
ボードのArduinoコア・SPI実装は開発環境側の依存です。

## サンプル

[examples](examples/README.md) にコントローラー3種類×試験モード4種類、計12個の試験サンプルを用意しています。
echo・周期送信と受信表示・シリアル入力送信と受信表示・単体診断の処理は共通で、
SPI初期化だけRP/ESPで分岐し、ボードごとのピンはスケッチ先頭で指定します。復旧方針はアプリ側へ追加します。

## 共通API

内蔵CANの設定例:

```cpp
#include <CANBridge.h>
#include <CANBridge/EspCan.h>
canbridge::Config config;
canbridge::Bus bus;

void setup() {
    Serial.begin(115200);
    config.bitrate = 1000000;
    config.txPin = D0;
    config.rxPin = D1;
    const auto result = bus.begin(config);
    if (result != canbridge::Result::Ok) Serial.println(canbridge::toString(result));
}
```

SPI接続の設定例:

```cpp
#include <CANBridge.h>
#include <CANBridge/Mcp2515.h>
canbridge::Config config;
canbridge::Bus bus;

void setup() {
    Serial.begin(115200);
    config.bitrate = 1000000;
    config.spi = &SPI;
    config.csPin = SS;
    config.oscillatorHz = 16000000;
    SPI.begin();
    const auto result = bus.begin(config);
    if (result != canbridge::Result::Ok) Serial.println(canbridge::toString(result));
}
```

MCP2518FDは `CANBridge/Mcp2518.h` を選びます。送受信は共通の
`bus.receive(frame)` / `bus.send(frame)` / `bus.pollHealth(health)` を使います。
Arduinoのライブラリ検出のため、共通入口 `CANBridge.h` を先にincludeし、
その次にコントローラー選択ヘッダーをincludeしてください。
1つの翻訳単位ではコントローラー選択ヘッダーを1つだけincludeしてください。
`CANBridge.h` は共通のFrame・Result・Health型を公開します。
同じアプリケーション内では同じコントローラー選択ヘッダーを使用してください。

必須項目はすべて未指定で初期化されます。通信速度・発振器周波数は0、ピンは-1、
SPIはnullptrが未指定です。通常モード（listenOnly=false）は任意項目の既定値です。
`begin(config)` は必須項目を順に検査し、最初の不足を具体的なResultと文字列で返します。
設定検査が通るまでドライバーの生成・ハードウェア初期化を行いません。
その実装に存在しない項目はConfig型にないためコンパイルエラーになります。
`SPI.begin()` 済みか、実際の配線・発振器が設定と一致するかは共通には検出できません。

`Frame` は `id / length / data[8] / extended / remote`。
SPIのピン設定と `SPI.begin()` はアプリ側で行います。外付け実装はポーリング方式です。
外付けのINTピンは使用せず、割り込み関数も不要です。
MCP2515は同期ポーリングです。ESP上のMCP2518FDは内部タスクを使い、
end()でそのタスクを終了します。
MCP2518FDの発振器は4/20/40 MHzを受け付けます。
ESP旧APIは125/250/500/1000 kbit/s、新APIと外付けは基盤のタイミング計算に従います。

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
`lossDetectionComplete` はfalseです。`receiveLoss=false` でも欠落がなかった保証にはなりません。
各値は現在状態・累積またはラッチで、読み出しても消去しません。イベント回数ではありません。
ライブラリはBus-offからの復旧処理やアプリ指令の再開判断を行いません。
コントローラー自身の復旧動作や保留中フレームの扱いはハードウェアに依存するため、
異常時に送信を止める方針はアプリ側で実装してください。
アプリが停止し、原因を解消して `end()` の成功を確認してから `begin(config)` で再開してください。
CANトランシーバーの電圧・終端・EN/STB固定配線を確認してください。
トランシーバー制御は初版に含めません。

## 開発と配布

ACAN2515 / ACAN2517FDをコミット固定のサブモジュールで読み込みます。
`src`の統合コードにより、ライブラリを1つ読み込むだけで依存がビルドされます。
ESPはArduinoコア同梱の公式ESP-IDF TWAIドライバーを直接使用します。
受信欠落の観測や送信バッファ寿命を保つため、別のESPラッパーは使用しません。
依存更新時はサブモジュールのコミット、ビルド、ライセンスをまとめて確認します。

仕様資料: [ESP TWAI](https://docs.espressif.com/projects/esp-idf/en/v5.5.3/esp32c5/api-reference/peripherals/twai.html)、
[ACAN2515](https://github.com/pierremolinaro/acan2515)、
[ACAN2517FD](https://github.com/pierremolinaro/acan2517FD)。

## ディレクトリ構成

```text
src/
  CANBridge.h            共通型を読み込む入口
  CANBridge/             公開ヘッダー（共通型とコントローラー選択）
  internal/              設定検査と共通Busの内部処理
  drivers/
    esp/                 ESP内蔵TWAIの実装
    mcp/                 MCP2515・MCP2518FDの実装
  dependencies/
    acan2515/            ACAN2515派生ソースとビルド入口
    acan2517FD/          ACAN2517FDソースのビルド入口
  ACAN*.h など           依存ヘッダーへの転送ファイル
examples/                ボード・コントローラー名を付けたサンプル
third_party/             元の依存ライブラリ（固定コミットのサブモジュール）
```

利用者は最初に `CANBridge.h` をincludeし、次に `CANBridge/EspCan.h`、`CANBridge/Mcp2515.h`、
`CANBridge/Mcp2518.h` のいずれかをincludeします。共通型だけなら `CANBridge.h` のみで使用できます。

`src` 直下の `ACAN*.h` と `MCP2515ReceiveFilters.h` は、サブモジュール内の
ヘッダーへ転送します。元ライブラリの山括弧形式のincludeをArduinoから解決するために
この位置に置いています。利用者が直接includeする必要はありません。
`dependencies/` は依存コードのビルド入口です。ACAN2515.cppは、固定した元ソースの
ESPタスク生成・セマフォ生成を除き、同期ポーリングへ変更した派生ソースを置いています。
元のACAN2515はESPの常駐タスクをend()で終了しないため、Busの終了・破棄後のアクセスを
防ぐための変更です。ACAN2517FD.cppも固定した元ソースの派生コピーとし、ポーリングと
割り込み禁止中の終了待ちに上限を設けています。サブモジュール本体には変更を加えていません。

## 起動前の共通診断

```cpp
canbridge::DiagnosticReport report;
const auto result = bus.diagnose(config, report);
// report.controllerAccess / controllerOperation / internalLoopback / cleanup
// 各項目: status (Passed/Failed/NotRun/Unsupported), reason, raw
// report.framesChecked: 内容が一致した試験フレーム数
if (result == canbridge::Result::Ok) bus.begin(config);
```

診断は停止中に限る同期処理です。通常と同じ設定検査・通信速度を使用し、
listenOnlyは試験中のみ無効にします。稼働中や終了未確認の場合はAlreadyStartedです。
設定検査の失敗・ドライバーオブジェクト確保失敗は試験前に返し、項目はNotRunです。
各ループバックフレームの送信受付・受信待ちは最大250 ms、4フレームで最大約1秒に、
ドライバー初期化・終了の時間が加わります。基盤のSPI呼び出しや共有SPIのロックが
戻らない場合までハードな実時間上限を保証するものではありません。
失敗後も終了処理を実行し、終了失敗は戻り値で優先して報告します。
終了成功時のみ所有ドライバーを解放します。終了失敗時は保持し、end()で再試行してください。
稼働中・終了未確認のインスタンスを破棄しない既存の利用条件は診断にも適用されます。

ESPは診断中だけ同じTX GPIOを送受信に使用し、ACK不要の自己受信を行います。
外部配線は不要ですがTX GPIOに信号は出ます。診断時は未接続のピンを使用してください。
通常RXピンや外部CAN回路は検証しません。ESPのaccessはSDKによる初期化成功の意味です。
MCPはレジスター/RAMアクセスと内部ループバックを利用します。
内部自己試験の成功は実際の通信速度・発振器の正しさを保証しません。
外部通信はexamplesの表示で人が確認します。

ホスト側の入力プロトコル・診断ライフサイクル試験は `python3 tests/host/run.py` で実行できます。
