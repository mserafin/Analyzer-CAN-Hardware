#pragma once

class ByteUtils
{
  public:
    static byte *slice(const byte *bufferSrc, const uint8_t index, const uint8_t bufferSize) {
      byte *bytes = new byte[bufferSize];
      for (uint8_t i = 0; i < bufferSize; i++) {
        bytes[i] = bufferSrc[i + index];
      }
      return bytes;
    }

    static uint16_t crc(const uint8_t *buffer, uint8_t bufferSize) {
      uint16_t crc = 0x00;
      while (bufferSize--) {
        uint8_t extract = *buffer++;
        for (uint8_t i = 8; i; i--) {
          uint8_t sum = (crc ^ extract) & 0x01;
          crc >>= 1;
          if (sum) {
            crc ^= 0x8C;
          }
          extract >>= 1;
        }
      }
      return crc;
    }
};
