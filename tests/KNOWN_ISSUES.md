# 実際に遭遇した問題

## 2026-10-11: GitHub ZIPをArduino IDEへ導入後、コンパイル失敗

- ユーザー報告: ZIPダウンロード版をArduino IDEへ導入するとコンパイルエラー。
- 推定原因: サブモジュールの実体が含まれていなかった可能性。該当エラーログは未取得。
- リポジトリの依存: third_party/acan2515 と third_party/acan2517FD はGitサブモジュール。
- 導入手順: `git clone --recurse-submodules https://github.com/KARURA-project/CANBridge.git`。
- 通常クローン済みなら、そのリポジトリ内で `git submodule update --init --recursive`。
- GitHub Download ZIPはGit管理情報もサブモジュールの中身も持たないため、
  再帰クローン版へ置き換える。配布ZIPを使う場合は依存ソースを同梱したものを使用する。
- READMEのArduino IDE向け導入説明を補強。ZIP導入失敗の解消確認は未取得。

## 2026-10-11: WindowsがESP32用コンパイラーの実行を拒否

- ユーザー提供エラー: `riscv32-esp-elf-g++.exe: An Application Control policy has blocked this file.`
- 対象: Arduino15/packages/esp32/tools/esp-rv32/2601/bin のコンパイラー。
- アプリ実行制限による起動失敗。CANBridgeソースのコンパイルエラーとは区別する。
- Smart App Controlまたは管理ポリシーの特定、対処、解消確認は未実施。

## 2026-10-11: Windowsで診断のシリアル表示なし、Macでは表示

- Windowsでコンパイル・書き込みした同一ボードを、再書き込みせずMacへ接続すると診断成功ログを表示。
- ユーザー報告: setup()でSerial接続待ちを追加するとWindowsでも表示するようになった。
- 診断自体の失敗ではなく、起動時のUSBシリアル接続・表示タイミングに関連する事象と判断。
- ボード型番、コア版、Windows/IDE版、実際に追加した待機コード全文は未取得。
- 全12サンプルとC5両基試験スケッチへ最大5秒のSerial接続待ち（delay(1)）を追加。
  実際の解消報告と、今回追加した全構成のコードの実機確認は区別する。
- 無期限に待たず、PC・モニターなしでも5秒後にはCAN処理を開始する。
