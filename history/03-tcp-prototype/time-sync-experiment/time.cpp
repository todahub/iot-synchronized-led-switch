bool readAndParseMessage(uint8_t buffer[],Client& client, ProtocolMessage& msg) {
  const size_t MSG_SIZE = 8;
  size_t bytesRead = 0;
  unsigned long start = millis();

  // まず8バイト読む
  while (bytesRead < MSG_SIZE) {
    if (client.available()) {
      buffer[bytesRead++] = client.read();
    }
    if (millis() - start > 1000) {
      Serial.println("Timeout while reading message");
      client.stop();
      return false;
    }
  }

  // パースしてみる
  ParseResult result = Parser::tryParseMessageFromBuffer(buffer, MSG_SIZE, msg);
  if (result == ParseResult::OK) return true;

  // フラグ異常の場合、再同期を試みる
  if (result == ParseResult::UnknownFlag) {
    Serial.println("Unknown flag, trying resync");

    int flagIndex = findValidFlagIndex(buffer, MSG_SIZE);
    if (flagIndex = -1) {
      Serial.println("No valid flag found, discarding buffer");
      return false;
    }

    // フラグ以降を前に詰める
    size_t remain = MSG_SIZE - flagIndex;
    for (size_t i = 0; i < remain; ++i) {
      buffer[i] = buffer[flagIndex + i];
    }

    // 残りを補完読み込み
    bytesRead = remain;
    start = millis(); // タイムアウト再設定

    while (bytesRead < MSG_SIZE) {
      if (client.available()) {
        buffer[bytesRead++] = client.read();
      }
      if (millis() - start > 1000) {
        Serial.println("Timeout during resync");
        client.stop();
        return false;
      }
    }

    // 再パース
    result = Parser::tryParseMessageFromBuffer(buffer, MSG_SIZE, msg);
    if (result == ParseResult::OK) return true;
    Serial.print("Parse failed after resync: ");
    Serial.println(static_cast<int>(result));
    return false;
  }

  // その他エラー（CRCなど）
  Serial.print("Parse failed: ");
  Serial.println(static_cast<int>(result));
  return false;
}





int findValidFlagIndex(const uint8_t* buffer, size_t len) {
  for (size_t i = 0; i < len; ++i) {
    uint8_t b = buffer[i];
    if (b == Constants::F_BTN || b == Constants::F_REQ ||
        b == Constants::F_RES || b == Constants::F_SYNC_R ||
        b == Constants::F_SYNC_S) {
      return i;
    }
  }
  return -1; // フラグが見つからない
}
