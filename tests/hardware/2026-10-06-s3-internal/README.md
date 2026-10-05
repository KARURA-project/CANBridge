# S3内蔵CAN・単体診断

実施日: 2026-10-06。実行者: ユーザー。証拠: 転送された[シリアルログ](serial.log)。
直前に案内したS3試験への返信として記録。
対象: feature/unified-diagnosticsの未コミット実装、基準HEAD 9e3f50e。
使用スケッチ（案内）: examples/InternalCanDiagnostic/InternalCanDiagnostic.ino。
ボード（申告された所持構成）: CANトランシーバー実装済みXIAO ESP32S3モジュール。
案内した条件: USB接続、CANH/CANLに相手なし、1 Mbit/s、TXをトランシーバーTXD接続GPIOに設定。
実際のピン設定・設定変更の有無・Arduinoコア/SDKバージョン・チップリビジョンは未確認。

結果: access/operation/loopback/cleanupすべてPassed、全体Ok、照合4フレーム。
単体診断1回の成功として記録。通常RX配線・トランシーバー・外部CAN通信、
通常起動や異常時復旧、反復試験は未検証。

[source-sha256.txt](source-sha256.txt)は記録時のローカル実装の識別情報。
ユーザーの書き込み済みバイナリーや変更後スケッチとの同一性を独立に確認したものではありません。
