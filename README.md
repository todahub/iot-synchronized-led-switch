# 独自プロトコルで複数のLEDを同調制御する IoT スイッチ

ハッカソン（チーム7）で、Arduino UNO R4 WiFi を使って製作した IoT システムです。
どれか1台のタクトスイッチを押すと、**すべてのクライアントのLEDが同じ時刻に切り替わる**ことを目的としています。

## 課題と方針

ルーターのファイアウォールにより、サーバーからクライアントへ能動的に通信できない環境を想定しました。
そのため次の方針を採用しています。

- クライアントがサーバーへ定期的に問い合わせる**ポーリング**でLED状態を取得する
- サーバーは「LEDの状態」と「切り替え時刻（ボタン押下時刻 + 1000 ms）」を返し、クライアントはその時刻まで待ってから切り替える
- クライアントとサーバーの時計は**NTP型の双方向ハンドシェイク**で同期する
- アプリケーション層に**8バイト固定長の独自プロトコル**（CRC-16-CCITT付き）を実装する。TCP・UDPのどちらでも使える

## システム構成

```
 [Client x3]  --(TCP)  ボタン押下通知 ---------------> [Server]
 タクトスイッチ + LEDマトリクス  <--(UDP)  LED状態 + 切替時刻 -- (LED状態を管理)
              --(UDP)  LED状態リクエスト / 時刻同期 -->
```

全体図は [docs/figures/システム全体の構成図.pdf](docs/figures/システム全体の構成図.pdf) を参照してください。

### 独自プロトコル（8 byte）

| flag (1B) | timestamp (4B) | data (1B) | CRC-16 (2B) |
|---|---|---|---|

| flag | 意味 |
|---|---|
| `0xF1` | ボタン押下通知 |
| `0xF2` | LED状態リクエスト |
| `0xF3` | LED制御レスポンス |
| `0xF4` / `0xF5` | 時刻同期リクエスト / レスポンス |

CRCは flag〜data の6バイトに対して計算します。受信側でCRC不一致とデータ欠損を検出し、不正なバイト列からはフラグ位置を探して再同期します。
関数仕様は [docs/protocol-spec/](docs/protocol-spec/) にあります。

## 最終版コード

| 場所 | 内容 |
|---|---|
| [firmware/client/](firmware/client/) | クライアント（チャタリング対策、UDPポーリング、時刻同期、LED切替） |
| [firmware/server/](firmware/server/) | サーバー（LED状態管理、切替時刻の決定、時刻同期応答） |

Arduino IDE で開いて書き込めます。対象ボードは Arduino UNO R4 WiFi です。
Wi-Fi の SSID とパスワードは `YOUR_SSID` / `YOUR_PASSWORD` に置き換えてあります。IPアドレスは書き込み先のネットワークに合わせて変更してください。

## 評価結果

測定データは [evaluation/](evaluation/) にあり、集計とグラフは `graf.ipynb` で再現できます。

| 測定 | 結果 |
|---|---|
| ボタン押下からLED切替までの時間（切替遅延を1秒に設定、n=60） | 平均 1.09 s、標準偏差 0.14 s |
| UDPの応答時間（n=11） | 平均 83 ms、標準偏差 53 ms |

時間同期誤差の推移は [docs/figures/時間同期誤差.png](docs/figures/時間同期誤差.png) にあります。

## 試行錯誤の履歴

最終版にたどり着くまでの過程を [history/](history/) に残しています。

| 段階 | 内容 |
|---|---|
| [01-led-matrix-basics](history/01-led-matrix-basics/) | LEDマトリクスの表示・点滅、Processingへのシリアル出力など基礎の練習 |
| [02-adc-speed-and-wifi-http](history/02-adc-speed-and-wifi-http/) | ADCの速度測定、クロック分周の実験、Wi-Fi接続とHTTP通信の試作 |
| [03-tcp-prototype](history/03-tcp-prototype/) | 独自プロトコルをTCPで実装した最初のサーバー・クライアントと、時刻同期の実験 |
| [04-udp-polling](history/04-udp-polling/) | ポーリングをUDPに切り替えた版（応答受信のバッファ処理、チャタリング対策を追加） |

## ドキュメント

- [docs/figures/](docs/figures/) : 構成図、通信シーケンス、時間同期の処理図
- [docs/diagrams/](docs/diagrams/) : draw.io / Mermaid の元データ
