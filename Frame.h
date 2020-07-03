#pragma once

enum FrameType
{
  CONFIG_UART = 0x01,
  CONFIG_CAN = 0x02,
  SYNC_TIME = 0x03,
  FRAME_STANDARD = 0x04,
  FRAME_EXTENDED = 0x05
};

class Frame
{
    class FrameBuilder
    {
      private:
        byte type;
        byte *data;
        byte dataSize;
        uint32_t canId;

        void fillType(byte *buffer, byte *cursor) {
          buffer[*cursor] = this->type;
        }

        void fillCanId(byte *buffer, byte *cursor) {
          byte *data = Convert::intToBytes(this->canId);
          for (byte i = 0; i < 4; i++) {
            buffer[*cursor += 1] = data[i];
          }
          delete data;
        }

        void fillDataSize(byte *buffer, byte *cursor) {
          buffer[*cursor += 1] = this->dataSize;
        }

        void fillData(byte *buffer, byte *cursor) {
          for (byte i = 0, l = this->dataSize; i < l; i++) {
            buffer[*cursor += 1] = this->data[i];
          }
        }

        void fillCrc(byte *buffer, byte bufferSize) {
          buffer[bufferSize] = ByteUtils::crc(buffer, bufferSize);
        }

      public:
        Frame *with(byte *data)
        {
          delete this;
          return new Frame(data);
        }

        FrameBuilder *withType(byte type) {
          this->type = type;
          return this;
        }

        FrameBuilder *withCanId(uint32_t canId)
        {
          this->canId = canId;
          return this;
        }

        FrameBuilder *withData(byte *data, byte dataSize)
        {
          this->data = data;
          this->dataSize = dataSize;
          return this;
        }

        byte *build()
        {
          byte bufferCursor = 0;
          byte bufferSize = 15;
          byte *buffer = new byte[bufferSize];

          this->fillType(buffer, &bufferCursor);
          this->fillCanId(buffer, &bufferCursor);
          this->fillDataSize(buffer, &bufferCursor);
          this->fillData(buffer, &bufferCursor);
          this->fillCrc(buffer, bufferSize - 1);
          delete this;

          return buffer;
        }
    };

  private:
    struct State {
      byte type;
      byte dataSize;
      byte* data;
      uint32_t canId;
      byte isExtended;
    } frame;

    bool _isValid;

    void configFrame(byte *buffer, byte type) {
      frame.canId = 0;
      frame.dataSize = buffer[1];
      frame.data = ByteUtils::slice(buffer, 2, frame.dataSize);
    }

    void dataStandardFrame(byte *buffer) {
      frame.isExtended = false;
      dataFrame(buffer);
    }

    void dataExtendedFrame(byte *buffer) {
      frame.isExtended = true;
      dataFrame(buffer);
    }

    void dataFrame(byte *buffer) {
      frame.dataSize = buffer[5];

      byte *bytes = ByteUtils::slice(buffer, 1, 4);
      frame.canId = Convert::bytesToInt(bytes, 4);
      frame.data = ByteUtils::slice(buffer, 6, frame.dataSize);

      delete bytes;
    }

    byte calcCRC(byte *buffer) {
      byte frameSize = getSize(buffer) - 1;;
      byte *frame = ByteUtils::slice(buffer, 0, frameSize);

      byte crc1 = ByteUtils::crc(frame, frameSize);
      byte crc2 = buffer[frameSize];

      delete frame;
      return crc1 == crc2;
    }

    byte getSize(byte *buffer) {
      switch (buffer[0]) {
        case 0x01:
        case 0x02:
        case 0x03: return buffer[1] + 3;
        case 0x04:
        case 0x05: return buffer[5] + 7;
      }
    }

  public:
    Frame(byte *buffer) {
      frame.type = (FrameType)buffer[0];
      if (!this->calcCRC(buffer)) {
        return;
      }

      this->_isValid  = true;

      switch (frame.type) {
        case 0x01:
        case 0x02:
        case 0x03: this->configFrame(buffer, frame.type); break;
        case 0x04: this->dataStandardFrame(buffer); break;
        case 0x05: this->dataExtendedFrame(buffer); break;
      }
    }

    ~Frame() {
      delete[] frame.data;
    }

    static FrameBuilder* builder()
    {
      return new FrameBuilder();
    }

    byte type() const {
      return frame.type;
    }

    uint32_t canId() const {
      return frame.canId;
    }

    byte* data() const {
      return frame.data;
    }

    byte data(byte index) const {
      return frame.data[index];
    }

    byte dataSize() const {
      return frame.dataSize;
    }

    bool isExtended() const {
      return frame.isExtended;
    }

    bool isValid() const {
      return _isValid;
    }
};
