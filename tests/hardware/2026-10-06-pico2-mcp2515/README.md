# Pico 2 + MCP2515・単体診断

実施日: 2026-10-06。実行者: ユーザー。証拠: 転送された[成功ログ](serial.log)。
対象: feature/unified-diagnosticsの未コミット実装、基準HEAD 9e3f50e。
直前に案内したPico 2試験への返信として記録。
モジュール: MCP2515、水晶8 MHz（ユーザー申告）。電源3.3 V（ユーザー申告）。
配線: MISO GP0、CS GP1、SCK GP2、MOSI GP3（ユーザーのRX/TX表記をSPI信号として解釈）。
使用スケッチ（案内）: examples/Mcp2515Diagnostic/Mcp2515Diagnostic.ino。
設定（案内）: oscillatorHz=8000000、bitrate=500000、config.spi=&SPI。
シリアル接続待機とBOOT表示を追加。変更後スケッチ全文は未取得。
ボード/コア/SDKバージョン、モジュール商品名・トランシーバー型番は未確認。

結果: access/operation/loopback/cleanupすべてPassed、全体Ok、照合4フレーム。
単体診断1回の成功として記録。外部通信・トランシーバーの3.3 V動作・実際の通信速度・
反復試験・通常起動・断線時復旧は未検証。

初回は速度変更忘れでraw=0x00000002（kTooFarFromDesiredBitRate）、operationがFailed。
[設定変更前ログ](initial-error.log)ではアクセス・終了がPassed、ループバックはNot run。
ユーザーが速度変更忘れを確認し、500 kbpsへの変更後に成功ログを提供しました。
この結果は設定エラーで診断が戻ることの確認であり、物理断線試験ではありません。

[source-sha256.txt](source-sha256.txt)は記録時のローカル実装の識別情報。
ユーザーの変更済みスケッチ・書き込み済みバイナリーとの同一性を独立に確認したものではありません。
