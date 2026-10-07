# Pico 2 + MCP2518・単体診断

実施日: 2026-10-07。実行者: ユーザー。証拠: 転送された[シリアルログ](serial.log)。
案内した試験への返信として記録。記録時のローカルHEAD: 942869530e5c7f9304f51666b198eca72e6dd885。
モジュール: JP WORKS MCP2518 + MCP2562FD、Switch Science商品10019、水晶20 MHz。
[メーカー資料](https://github.com/TLDSJPWORK/CAN-FD_Board)。
使用スケッチ（案内）: examples/Mcp2518Diagnostic/Mcp2518Diagnostic.ino。
案内設定: Pico 2、500 kbps、oscillatorHz=20000000、SO GP0、CS GP1、SCK GP2、SI GP3。
ジャンパーは3.3 V側、5V端子にVBUS、3.3V端子に3V3 OUT、GND共通。
INT/CANH/CANL未接続、終端両方OFF。Serial接続待ちとBOOT表示を追加。
実際の変更済みスケッチ全文・配線の独立確認・コア版・チップリビジョンは未取得。

結果: access/operation/loopback/cleanupすべてPassed、全体Ok、照合4フレーム。
単体診断1回の成功として記録。外部通信・実際の水晶周波数/ビットレート・
反復試験・通常起動・異常時復旧は未検証。
[source-sha256.txt](source-sha256.txt)は記録時のローカル実装識別情報で、
ユーザーの変更済みスケッチや書き込み済みバイナリーとの同一性を独立に確認したものではありません。
