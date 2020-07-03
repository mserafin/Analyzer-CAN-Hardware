#pragma once

const byte SPI_CS_PIN = 10;

MCP_CAN CAN(SPI_CS_PIN);

enum Mode {
  RX     = 0x01,
  TX     = 0x02,
  DUPLEX = 0x03
};

//enum Protocol {
//  CAN = 0x01,
//  OBD = 0x02
//};

typedef void (*ConnCanDataCallback)(byte *data, byte dataSize);

class ConnCAN //SnifferCan
{
  private:
    BaudRate baudRate;
    ConnCanDataCallback dataCallback;

  public:
    void begin(ConnCanDataCallback dataCallback) {
      this->dataCallback = dataCallback;
    }

    void refresh() {
      if (CAN_MSGAVAIL == CAN.checkReceive())
      {
        byte size = 0;
        byte buffer[8];
        CAN.readMsgBuf(&size, buffer);

        byte *frame = Frame::builder()
                      ->withType(FRAME_EXTENDED)
                      ->withCanId(CAN.getCanId())
                      ->withData(buffer, size)
                      ->build();

        this->dataCallback(frame, 15);
        delete frame;
      }
    }

    bool setBaudRate(uint16_t baudrate) {
      return CAN.begin(baudrate) == CAN_OK;
    }

    bool sendMsg(Frame *frame) {
      return CAN.sendMsgBuf(frame->canId(),
                            frame->isExtended(),
                            frame->dataSize(),
                            frame->data()) == CAN_OK;
    }
};
