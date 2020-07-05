#pragma once

typedef void (*RequestConfigCallback)(const FrameType type, const uint32_t value);

typedef void (*RequestDataCallback)(const Frame* frame);

class Request
{
  private:
    UARTConfig* uart;
    RequestConfigCallback configCallback;
    RequestDataCallback dataCallback;

  public:
    Request(UARTConfig* uart) {
      this->uart = uart;
    }

    void begin(RequestConfigCallback configCallback,
               RequestDataCallback dataCallback)
    {
      this->configCallback = configCallback;
      this->dataCallback = dataCallback;
      this->setBaudRate(this->uart->baudrate);
    }

    void refresh()
    {
      if (Serial.available())
      {
        byte buffer[FRAME_LENGTH];
        Serial.setTimeout(this->uart->maxTimeOut);
        Serial.readBytes(buffer, FRAME_LENGTH);

        Frame *frame = Frame::builder()->with(buffer);

        if (frame->isValid()) {
          switch (frame->type()) {
            case 0x01:
            case 0x02:
            case 0x03:
              this->configCallback(
                (FrameType)frame->type(), Convert::bytesToInt(frame->data(), frame->dataSize()));
              break;
            case 0x04:
            case 0x05:
              this->dataCallback(frame);
              break;
          }
        }
        delete frame;
      }
    }

    void setBaudRate(uint32_t baudrate) {
      Serial.begin(baudrate, SERIAL_8N1);
    }
};
