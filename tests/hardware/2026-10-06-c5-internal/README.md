# C5内蔵CAN・単体診断

実施日: 2026-10-06。実行者: ユーザー。証拠: 転送された[シリアルログ](serial.log)。
対象: feature/unified-diagnosticsの未コミット実装、基準HEAD 9e3f50e。
使用スケッチ: examples/InternalCanDiagnostic/InternalCanDiagnostic.ino。
案内した設定: 1 Mbit/s、TX D0、RX D1、GPIO未接続のXIAO ESP32C5。
実際の設定変更の有無・コア/SDKバージョン・チップリビジョンは未確認。

結果: access/operation/loopback/cleanupすべてPassed、全体Ok、照合4フレーム。
単体診断1回の成功として記録。複数回起動・同一プロセスでの再初期化・
異常時の復旧・通常RX配線・外部CAN通信は未検証。

[source-sha256.txt](source-sha256.txt)は記録時のローカル実装の識別情報。
ユーザーの書き込み済みバイナリーとの同一性を独立に確認したものではありません。
