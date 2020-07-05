#pragma once

enum Mode {
  RX     = 0x01,
  TX     = 0x02,
  DUPLEX = 0x03
};

enum Protocol {
  CAN = 0x01,
  OBD = 0x02
};

typedef void (*CANSnifferDataCallback)(const byte* data, const byte dataSize);

class CANSniffer : private MCP_CAN
{
  private:
    BaudRate baudRate;
    CANSnifferDataCallback dataCallback;

  public:
    CANSniffer(byte spi_cs_pin) : MCP_CAN(spi_cs_pin) {

    }

    void begin(CANSnifferDataCallback dataCallback) {
      this->dataCallback = dataCallback;
    }

    void refresh() {
      if (CAN_MSGAVAIL == checkReceive()) {
        byte size = 0;
        byte buffer[8];
        readMsgBuf(&size, buffer);

        byte* frame = Frame::builder()
                      ->withType(FRAME_EXTENDED)
                      ->withCanId(getCanId())
                      ->withData(buffer, size)
                      ->build();

        dataCallback(frame, FRAME_LENGTH);
        delete[] frame;
      }
    }

    bool setBaudRate(uint16_t baudrate) {
      return MCP_CAN::begin(baudrate) == CAN_OK;
    }

    bool sendMsg(const Frame* frame) {
      return sendMsgBuf(frame->canId(),
                        frame->isExtended(),
                        frame->dataSize(),
                        frame->data()) == CAN_OK;
    }

    void enableFilterOBD()
    {
      init_Mask(0, 0, 0x7FC);
      init_Mask(1, 0, 0x7FC);

      init_Filt(0, 0, 0x7E8);
      init_Filt(1, 0, 0x7E8);

      init_Filt(2, 0, 0x7E8);
      init_Filt(3, 0, 0x7E8);
      init_Filt(4, 0, 0x7E8);
      init_Filt(5, 0, 0x7E8);
    }
};
