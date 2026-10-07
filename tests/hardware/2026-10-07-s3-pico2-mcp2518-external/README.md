# S3内蔵CAN ↔ Pico 2 + MCP2518・外部往復通信

実施日: 2026-10-07。ユーザー転送ログによる確認。記録時HEAD: 9428695。
案内構成: S3 InternalCanPeriodic（TX D0/RX D1）とPico 2 Mcp2518Echo。
双方500 kbps、Pico 2のMCP2518水晶20 MHz、SPI SO GP0/CS GP1/SCK GP2/SI GP3。
CANH同士/CANL同士/GND共通、両端終端有効の配線を案内。
S3トランシーバー型番・実行コア版・変更後スケッチ全文は未確認。

結果: [S3ログ](s3-serial.log)で標準ID 0x123、4バイト連番0〜10の11フレームについて
TX acceptedと同一データのRXを確認。案内したPeriodic/Echo構成での外部往復通信成功として記録。
[Pico 2ログ](pico2-serial.log)は起動成功・health異常なし。
現行Echoモードはフレームログを出さないため、起動/healthのみの表示は仕様通り。
両ログのhealthはbusOff=0/passive=0/receiveLoss=0。これらは表示時点の値。

ユーザーはS3と思われるログを2回貼り付け、2回目末尾RXのみ1桁欠落。
保存したS3ログは最初の完全なブロック。Pico 2ログは末尾ブロック。
長時間運転、逆方向Periodic、
異常時復旧は未実施。TX acceptedだけを到達証拠にはしていない。

## 相手切断後の受信停止

ユーザーへPico 2のUSBのみを抜く試験を案内し、[切断試験ログ](disconnect-serial.log)を取得。
19:21:39〜43は連番0x85〜0x89でTX/RXが一致。
19:21:44〜54は連番0x8A〜0x94のTX acceptedのみ、RXなし（11送信受付）。
相手切断後に往復が停止することを確認。ログ範囲にhealth異常の追加表示なし。
再接続後の結果は下記。bus-off時の挙動は未確認。

## 相手再接続後の往復再開

S3をリセットせずPico 2を再接続してモニターを開く手順への返信として
[再接続ログ](reconnect-serial.log)を取得。
19:22:14〜22は0xA8〜0xB0のTX acceptedのみ、19:22:23〜26は0xB1〜0xB4でTX/RXが一致。
この試験条件で相手再接続後の往復再開を確認。S3再起動や明示的復旧操作は案内していない。
復帰時healthはbusOff=0/passive=0/receiveLoss=0、raw=0x27。
ESPバックエンドのhealth.rawはバスエラー累積件数（0x27=39）、状態ビットではない。
切断中のACK不在等に伴うエラーと整合するが、この値だけではエラー種別を特定できない。
bus-off到達後の復旧・診断後のbegin/end再起動は検証していない。

## シリアル入力と各フレーム種別の往復

S3をInternalCanSerialInputへ変更し、Pico 2はMcp2518Echoのまま使う手順への返信として
[シリアル入力ログ](serial-input-serial.log)を取得。双方500 kbps、S3 TX D0/RX D1。
以下の6ケースすべてでTX acceptedと同じ内容のRXを確認。

| 種別 | ID | 内容 |
| --- | --- | --- |
| 標準データ | 0x123 | 1122（2バイト） |
| 標準データ | 0x123 | 空（0バイト） |
| 標準データ | 0x123 | 0011223344556677（8バイト） |
| 拡張データ | 0x001ABCDE | 0102030405060708（8バイト） |
| 標準RTR | 0x123 | DLC 8 |
| 拡張RTR | 0x001ABCDE | DLC 0 |

RTRは相手が同じRTRフレームをEchoする試験。要求に対するデータ応答プロトコルの検証ではない。
入力解析、S3送信、MCP2518受信・同一フレーム返信、S3受信・表示の往復を確認。
起動時healthはbusOff=0/passive=0/receiveLoss=0。各ケース1回、長時間・高負荷試験は未実施。
