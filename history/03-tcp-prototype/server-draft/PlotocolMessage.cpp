#include "ProtocolMessage.h"

namespace Protocol {

  namespace CRC {
    // CRC-16-CCITT の計算関数（バイト列に対してCRC値を算出）
    uint16_t calculate(const uint8_t* data, size_t length) {
      uint16_t crc = Constants::CRC_INITIAL;
      for (size_t i = 0; i < length; ++i) {
        crc ^= (uint16_t)data[i] << 8;
        for (uint8_t j = 0; j < 8; ++j) {
          crc = (crc & 0x8000) ? (crc << 1) ^ Constants::CRC_POLY : (crc << 1);
        }
      }
      return crc;
    }
  }

  // コンストラクタ（CRCは0で初期化）
  ProtocolMessage::ProtocolMessage(uint8_t f, uint32_t ts, uint8_t d)
    : flag(f), timestamp(ts), data(d), crc(0) {}

  // デフォルトコンストラクタ（全要素を0に初期化）
  ProtocolMessage::ProtocolMessage()
    : flag(0), timestamp(0), data(0), crc(0) {}

  // バイト列からメッセージに変換
  void ProtocolMessage::fromBytes(const uint8_t* buf) {
    flag = buf[0];
    timestamp = ((uint32_t)buf[1] << 24) | ((uint32_t)buf[2] << 16) |
                ((uint32_t)buf[3] << 8) | buf[4];
    data = buf[5];
    crc = ((uint16_t)buf[6] << 8) | buf[7];
  }

  // メッセージをバイト列に変換（末尾にCRCを自動付加）
  void ProtocolMessage::toBytes(uint8_t* buf) const {
    buf[0] = flag;
    buf[1] = (timestamp >> 24) & 0xFF;
    buf[2] = (timestamp >> 16) & 0xFF;
    buf[3] = (timestamp >> 8) & 0xFF;
    buf[4] = timestamp & 0xFF;
    buf[5] = data;
    // CRCを再計算して末尾に格納
    uint16_t calculatedCRC = CRC::calculate(buf, Constants::PROTOCOL_CRC_SIZE);
    buf[6] = (calculatedCRC >> 8) & 0xFF;
    buf[7] = calculatedCRC & 0xFF;
  }

  namespace Factory {
    // ボタン押下メッセージ作成（flag=F_BTN, data=0x00）
    void createButtonPressMessage(uint8_t* buffer, uint32_t time) {
      ProtocolMessage msg(Constants::F_BTN, time, 0x00);
      msg.toBytes(buffer);
    }

    // LED状態リクエスト作成（flag=F_REQ）
    void createLEDStateRequest(uint8_t* buffer) {
      ProtocolMessage msg(Constants::F_REQ, 0x00000000, 0x00);
      msg.toBytes(buffer);
    }

    // LED制御レスポンス作成（flag=F_RES, ON/OFF指定可,trueがONfolseがOFF）
    void createLEDControlResponse(uint8_t* buffer, uint32_t time, bool turnOn) {
      ProtocolMessage msg(Constants::F_RES, time, turnOn ? Constants::LED_ON : Constants::LED_OFF);
      msg.toBytes(buffer);
    }

    // 時刻同期リクエスト作成（flag=F_SYNC_R, t1はリクエストを送る時間,data=0）
    void createTimeSyncRequest(uint8_t* buffer, uint32_t t1) {
      ProtocolMessage msg(Constants::F_SYNC_R, t1, 0x00);
      msg.toBytes(buffer);
    }

    // 時刻同期レスポンス作成（flag=F_SYNC_S, data=差分=t2-t1）
    void createTimeSyncResponse(uint8_t* buffer, uint32_t t3, uint8_t diff) {
      ProtocolMessage msg(Constants::F_SYNC_S, t3, diff);
      msg.toBytes(buffer);
    }
  }

  namespace Parser {
    // バッファからメッセージを解析して構造体に展開
    ParseResult tryParseMessageFromBuffer(const uint8_t* buffer, size_t len, ProtocolMessage& msg) {
      if (len < Constants::PROTOCOL_MSG_SIZE) return ParseResult::NotEnoughData; // データ不足(8バイト想定)

      // フラグが未知であればエラー（通信時にバイトが欠けた時の対策）
      uint8_t flag = buffer[0];
      if (flag != Constants::F_BTN && flag != Constants::F_REQ && flag != Constants::F_RES &&
          flag != Constants::F_SYNC_R && flag != Constants::F_SYNC_S) {
        return ParseResult::UnknownFlag;
      }

      msg.fromBytes(buffer); // バッファから構造体へ変換

      // CRCの整合性確認
      uint16_t expectedCRC = CRC::calculate(buffer, Constants::PROTOCOL_CRC_SIZE);
      if (expectedCRC != msg.crc) return ParseResult::CRCMismatch;

      return ParseResult::OK; // 正常に解析完了
    }

    MessageType getMessageType(const ProtocolMessage& msg) {
    //送られたメッセージのフラグを見て種類を判断
    switch (msg.flag) {
      case Constants::F_BTN:    return MessageType::ButtonPress;
      case Constants::F_REQ:    return MessageType::LEDRequest;
      case Constants::F_RES:    return MessageType::LEDResponse;
      case Constants::F_SYNC_R: return MessageType::TimeSyncRequest;
      case Constants::F_SYNC_S: return MessageType::TimeSyncResponse;
      default:                  return MessageType::Unknown;
    }
  }
  }


} // namespace Protocol
