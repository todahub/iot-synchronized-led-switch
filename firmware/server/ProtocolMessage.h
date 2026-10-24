#ifndef PROTOCOL_MESSAGE_H
#define PROTOCOL_MESSAGE_H

#include <Arduino.h>

namespace Protocol {

  // プロトコルに関する定数群
  namespace Constants {
    constexpr size_t PROTOCOL_MSG_SIZE  = 8;      // メッセージの総バイト数
    constexpr size_t PROTOCOL_CRC_SIZE  = 6;      // CRC計算に使用するバイト数（flag〜dataまで）
    constexpr uint16_t CRC_INITIAL      = 0xFFFF; // CRC初期値
    constexpr uint16_t CRC_POLY         = 0x1021; // CRC-16-CCITT の多項式

    constexpr uint8_t LED_OFF           = 0x00;   // LED消灯指令
    constexpr uint8_t LED_ON            = 0x01;   // LED点灯指令

    // フラグ定義：各種メッセージの種別を表す
    constexpr uint8_t F_BTN     = 0xF1; // ボタン押下通知
    constexpr uint8_t F_REQ     = 0xF2; // LED状態リクエスト
    constexpr uint8_t F_RES     = 0xF3; // LED制御レスポンス
    constexpr uint8_t F_SYNC_R  = 0xF4; // 時刻同期リクエスト
    constexpr uint8_t F_SYNC_S  = 0xF5; // 時刻同期レスポンス
  }

  // CRC計算用名前空間
  namespace CRC {
    uint16_t calculate(const uint8_t* data, size_t length); // CRC計算関数の宣言
  }

  // プロトコルメッセージ構造体
  struct ProtocolMessage {
    uint8_t flag;        // メッセージの種類を示すフラグ
    uint32_t timestamp;  // タイムスタンプ
    uint8_t data;        // データ領域（ON/OFFや時刻差など）
    uint16_t crc;        // CRC-16 チェックサム

    ProtocolMessage(uint8_t f, uint32_t ts, uint8_t d); // 初期化コンストラクタ
    ProtocolMessage();                                  // デフォルトコンストラクタ

    void fromBytes(const uint8_t* buf); // バイト列からメッセージへ展開
    void toBytes(uint8_t* buf) const;   // メッセージをバイト列へ変換（CRC計算込み）
  };

  // メッセージ作成ユーティリティ
  namespace Factory {
    void createButtonPressMessage(uint8_t* buffer, uint32_t time);           // ボタン押下メッセージ作成
    void createLEDStateRequest(uint8_t* buffer);                             // LED状態リクエストメッセージ作成
    void createLEDControlResponse(uint8_t* buffer, uint32_t time, bool turnOn); // LED制御レスポンス作成
    void createTimeSyncRequest(uint8_t* buffer, uint32_t t1);                // 時刻同期リクエスト作成
    void createTimeSyncResponse(uint8_t* buffer, uint32_t t3, uint8_t diff); // 時刻同期レスポンス作成
  }

  // メッセージ解析ユーティリティ
  namespace Parser {
    enum class ParseResult {
      OK,             // 正常に解析完了
      NotEnoughData,  // データが不足している
      UnknownFlag,    // 未知のフラグ
      CRCMismatch     // CRCチェック失敗
    };

    ParseResult tryParseMessageFromBuffer(const uint8_t* buffer, size_t len, ProtocolMessage& msg); // バッファからメッセージを解析(エラー検知)
  

      // メッセージの意味
    enum class MessageType {
        ButtonPress,
        LEDRequest,
        LEDResponse,
        TimeSyncRequest,
        TimeSyncResponse,
        Unknown
    };

    MessageType getMessageType(const ProtocolMessage& msg);
  }

} // namespace Protocol

#endif // PROTOCOL_MESSAGE_H

