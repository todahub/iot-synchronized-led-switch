
//このファイル入らなくなっちゃったからメモに使ってるよ

inline void handleIncomingMessage(const uint8_t* buffer) {
  ProtocolMessage msg;
  msg.fromBytes(buffer);

  uint16_t checkCRC = calculateCRC(buffer, 6);
  if (checkCRC != msg.crc) {
    Serial.println("CRC mismatch. Message invalid.");
    return 1;
  }

  switch (msg.flag) {
    case 0x01:
      Serial.print("Button pressed at time: ");
      Serial.println(msg.timestamp);
      break;
    case 0x02:
      Serial.println("Received LED state request");
      break;
    case 0x03:
      Serial.print("LED state should be: ");
      Serial.println((msg.data & 0x01) ? "ON" : "OFF");
      break;
    case 0x04:
      Serial.print("Time sync requested. T1: ");
      Serial.println(msg.timestamp);
      break;
    case 0x05:
      Serial.print("Time sync response. T3: ");
      Serial.println(msg.timestamp);
      Serial.print("Time diff: ");
      Serial.println(msg.data);
      break;
    default:
      Serial.println("Unknown flag.");
      break;
  }
}
