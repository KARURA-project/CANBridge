# C5内蔵CAN・両コントローラー診断

実施日: 2026-10-06。実行者: ユーザー。証拠: 転送された[シリアルログ](serial.log)。
対象: feature/unified-diagnosticsの未コミット実装、基準HEAD 9e3f50e。
使用スケッチ: [C5TwoControllers](../C5TwoControllers/C5TwoControllers.ino)。
ログで確認した環境: Arduino-ESP32 3.3.10、ESP-IDF v5.5.4。
案内した条件: XIAO ESP32C5単体、USB接続のみ、D0/D1/D2未接続。
スケッチ設定: 1 Mbit/s、TX D0、RX D1。予約ノードはD2を使用し、開始・送信しません。
実際の設定変更の有無・チップリビジョンは未確認。

結果: TWAI0/TWAI1ともaccess/operation/loopback/cleanupがPassed、全体Ok、各4フレーム照合。
TWAI0の予約・解放もESP_OK。各コントローラー1回の内部診断成功として記録。
SDKの空きスロット昇順割り当てを使い、TWAI0を予約中に次のノードを診断してTWAI1を確認。
同じBusオブジェクトで診断を再実行できたことも確認できました。
通常モードでのbegin()/end()、2バス同時通信、外部CAN通信、通常RX配線、異常時復旧は未検証。

[source-sha256.txt](source-sha256.txt)は記録時のローカル実装の識別情報です。
ユーザーの書き込み済みバイナリーとの同一性を独立に確認したものではありません。
