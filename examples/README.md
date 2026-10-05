# CANBridge試験サンプル

まずCANコントローラーの行と、試したい機能の列からサンプルを選んでください。
ボードごとのコピーはありません。各スケッチ先頭のピン・通信速度・水晶周波数を
実物に合わせて変更し、Arduino IDEで自分のボードを選んでビルドします。

| コントローラー | 受信をそのまま返信 | 周期送信＋受信表示 | 手入力送信＋受信表示 | 単体診断 |
| --- | --- | --- | --- | --- |
| InternalCan | [Echo](InternalCanEcho/InternalCanEcho.ino) | [Periodic](InternalCanPeriodic/InternalCanPeriodic.ino) | [SerialInput](InternalCanSerialInput/InternalCanSerialInput.ino) | [Diagnostic](InternalCanDiagnostic/InternalCanDiagnostic.ino) |
| Mcp2515 | [Echo](Mcp2515Echo/Mcp2515Echo.ino) | [Periodic](Mcp2515Periodic/Mcp2515Periodic.ino) | [SerialInput](Mcp2515SerialInput/Mcp2515SerialInput.ino) | [Diagnostic](Mcp2515Diagnostic/Mcp2515Diagnostic.ino) |
| Mcp2518 | [Echo](Mcp2518Echo/Mcp2518Echo.ino) | [Periodic](Mcp2518Periodic/Mcp2518Periodic.ino) | [SerialInput](Mcp2518SerialInput/Mcp2518SerialInput.ino) | [Diagnostic](Mcp2518Diagnostic/Mcp2518Diagnostic.ino) |

- InternalCan: XIAO ESP32S3 / ESP32C5。外部通信時はトランシーバーが必要です。
- Mcp2515 / Mcp2518: 上記ESPとXIAO RP2350 / Pico / Pico 2。
- RP系はArduino-Pico、ESP系はArduino-ESP32を使用してください。

## 開発を始める順序

1. Diagnosticでアクセス・動作・ループバック・終了を確認します。
2. 2台をCAN接続し、一方をPeriodic、もう一方をEchoにして往復を確認します。
3. SerialInputで必要なID・データを手入力して確認します。
4. 自分のアプリへ、選択ヘッダー・Config/Bus宣言・初期設定を移します。
   送受信はbus.send()/bus.receive()、状態確認はbus.pollHealth()を使います。

## 変更する場所

各スケッチは「設定値 → 共通宣言 → 初期化 → loop」の順です。
SPIのSCK/MISO/MOSI/CSは最初から明示しています。RPはsetSCK/setMISO/setMOSIの
戻り値を確認してからbegin()、ESPはbegin(SCK, MISO, MOSI)で設定します。
この数行だけをプリプロセッサで分岐し、送受信処理は共通です。
RPでは各SPIバスに対応するピンを選んでください。CSはCANドライバーが制御します。
Periodicはrunner.periodMsとrunner.periodicIdで周期・IDを変更できます。

Runnerはサンプル用の共通補助処理で、CANBridge/Examples.hにあります。
通常のアプリはRunnerを使わず、共通のBus APIを直接呼べます。

## Setup

Open the matching sketch, select its board and adjust bitrate/pins/crystal.
Internal CAN defaults to D0 TX / D1 RX. SPI uses the default SPI pins, SS,
16 MHz for MCP2515 and 40 MHz for MCP2518FD; call SPI.begin() before starting.
SPI diagnostic still requires the powered CAN module and its SPI wiring.
ESP diagnostic requires no external transceiver or jumper: it temporarily uses
TX GPIO for both TX/RX, then releases the diagnostic output. The normal RX pin
and external physical layer are not tested. Leave diagnostic TX unconnected.

Serial is 115200. No sketch waits for the serial monitor; open it before reset
to see startup output. The runner queues output and writes only when serial has
space. If output cannot keep up, whole log lines are dropped and LOG dropped=N
is reported. This is a low-rate connectivity test, not a lossless bus recorder.

## Protocol and display

Send one line, terminated by newline (CRLF also works):

```text
S 123#112233AABB
E 001ABCDE#01020304
S 123#
S 123#R8
```

S/E select standard/extended ID. Hex ID is 1..3/1..8 digits and within CAN
limits. Data is 0..8 bytes, two hex digits per byte; lowercase hex is accepted.
R0..R8 selects an RTR frame with the requested DLC and no data bytes. Input
buffer is 39 characters; overlong lines are rejected up to the next newline.
Only one transmission can be pending; another complete input line is rejected
while it is pending. Input and RX continue while TX returns Busy.

Both display examples use identical formatting:

```text
TX accepted S 123#112233AABB
RX S 123#112233AABB
RX E 001ABCDE#01020304
RX S 123#R8
```

IDs are uppercase and padded to 3/8 digits. Copy the part after RX into the input
example to resend it. TX accepted means queued, not physical delivery or ACK.
Health changes and operation errors are also logged. Echo does not print frames.

## External communication test

Use Periodic or SerialInput on one node and Echo on the other, with matching
bitrate, compatible transceivers, common ground and correct termination. Observe
TX and RX on the initiating node; no automatic external pass/fail is inferred.
Do not run echo on both nodes: frames will circulate indefinitely. Use a dedicated
test bus; echo and test frames can affect devices on a live actuator bus.

## Diagnostic interpretation

Passed loopback confirms the controller test path, not the transceiver, external
CAN wiring, normal RX pin, or accuracy of the configured oscillator frequency.
A wrong crystal setting may still pass internal loopback. Reported raw flags are
backend-specific. If cleanup fails, stop and correct the hardware; begin/diagnose
remain blocked until end() confirms shutdown. On success call begin(config)
separately to start normal operation. Tests cover standard/extended data frames,
zero/eight bytes and a remote frame. Runtime and memory tests on physical boards
remain required; these examples do not certify hardware functionality.
