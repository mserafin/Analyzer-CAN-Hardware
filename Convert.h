#pragma once

class Convert
{
  public:
    static uint32_t bytesToInt(byte *buffer, uint8_t bufferSize)
    {
      uint32_t accum = 0L;
      for (uint8_t i = 0; i < bufferSize; i++) {
        accum <<= 8;
        accum |= (buffer[i] & 0xFF);
      }
      return accum;
    }

    static byte *intToBytes(uint32_t value) {
      uint8_t bytesSize = 4;
      byte *bytes = new byte[bytesSize] {0, 0, 0, 0};
      for (int i = 0; i < bytesSize; i++) {
        bytes[bytesSize - 1 - i] = (byte) ((value >> i * 8) & 0xFF);
      }
      return bytes;
    }
};
