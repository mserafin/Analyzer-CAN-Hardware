#pragma once

typedef void (*ConnUartConfigCallback)(FrameType type, uint32_t value);

typedef void (*ConnUartDataCallback)(Frame* frame);

class ConnUART //SerialUart
{
  private:
    UARTConfig* uart;
    ConnUartConfigCallback configCallback;
    ConnUartDataCallback dataCallback;

  public:
    ConnUART(UARTConfig* uart) {
      this->uart = uart;
    }

    void begin(ConnUartConfigCallback configCallback,
               ConnUartDataCallback dataCallback)
    {
      this->configCallback = configCallback;
      this->dataCallback = dataCallback;
      this->setBaudrate(this->uart->baudrate);
    }

    void watching()
    {
      if (Serial.available())
      {
        byte buffer[this->uart->maxFrameSize];
        Serial.setTimeout(this->uart->maxTimeOut);
        Serial.readBytes(buffer, this->uart->maxFrameSize);

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

    void setBaudrate(uint32_t baudrate) {
      Serial.begin(baudrate, SERIAL_8N1);
    }
};
