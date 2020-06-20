#pragma once
#include "Frame.h"
#include "SPI.h"
#include "mcp_can.h"

const byte     SPI_CS_PIN     = 0x0A;
const uint16_t BAUD_RATE_SIZE = 0x11;

MCP_CAN CAN(SPI_CS_PIN);

//enum Mode {
//  RX     = 0x01,
//  TX     = 0x02,
//  DUPLEX = 0x03
//};

//enum Protocol {
//  CAN = 0x01,
//  OBD = 0x02
//};

struct BaudRate {
  uint16_t speed;
  String name;
};

typedef void (*ConnCanDataCallback)(byte *data, byte dataSize);
typedef void (*ConnCanBaudRateCallback)(BaudRate baudRate);

class ConnCAN //SnifferCan
{
  private:
    CANConfig* config;
    ConnCanDataCallback dataCallback;
    ConnCanBaudRateCallback baudRateCallback;

    struct BaudRate baudRate[BAUD_RATE_SIZE] =
    {
      {0, "UNKNOWN"},
      {CAN_1000KBPS, "CAN 1000"},
      {CAN_666KBPS,  "CAN 666"},
      {CAN_500KBPS,  "CAN 500"},
      {CAN_250KBPS,  "CAN 250"},
      {CAN_200KBPS,  "CAN 200"},
      {CAN_125KBPS,  "CAN 125"},
      {CAN_100KBPS,  "CAN 100"},
      {CAN_95KBPS,   "CAN 95"},
      {CAN_80KBPS,   "CAN 80"},
      {CAN_50KBPS,   "CAN 50"},
      {CAN_40KBPS,   "CAN 40"},
      {CAN_33KBPS,   "CAN 33"},
      {CAN_25KBPS,   "CAN 25"},
      {CAN_20KBPS,   "CAN 20"},
      {CAN_10KBPS,   "CAN 10"},
      {CAN_5KBPS,    "CAN 5"}
    };

    BaudRate getBaudRateBySpeed(uint16_t speed) {
      for (uint16_t i = 1, l = sizeof(this->baudRate); i < l; i++) {
        if (this->baudRate[i].speed == speed) {
          return this->baudRate[i];
        }
      }
      return this->baudRate[0];
    }

  public:
    ConnCAN(CANConfig *config) {
      this->config = config;
    }

    void begin(ConnCanDataCallback dataCallback,
               ConnCanBaudRateCallback baudRateCallback = NULL)
    {
      this->dataCallback = dataCallback;
      this->baudRateCallback = baudRateCallback;
      if (this->config->autoScan) {
        this->searchSpeed();
      }
    }

    void watching() {
      if (CAN_MSGAVAIL == CAN.checkReceive())
      {
        byte size = 0;
        byte buffer[8];
        CAN.readMsgBuf(&size, buffer);

        Serial.println(CAN.getCanId());
        for (int i = 0; i < size; i++)
        {
          Serial.print(buffer[i]);
          Serial.print(",");
        }
        Serial.println();

        byte *frame = Frame::builder()
                      ->withType(FRAME_EXTENDED)
                      ->withCanId(CAN.getCanId())
                      ->withData(buffer, size)
                      ->build();

        this->dataCallback(frame, 15);
        delete frame;
      }
    }

    bool searchSpeed() {
      byte count = this->config->searchCount;

      do {
        for (uint16_t i = 1; i < BAUD_RATE_SIZE; i++) {
          if (this->setSpeed((this->baudRate[i]).speed))
          {
            //TODO: Check if data is flowing!
            return true;
          }
          delay(100);
        }
      } while (count--);

      return false;
    }

    bool setSpeed(uint16_t speed) {
      if (CAN_OK == CAN.begin(speed)) {
        if (this->baudRateCallback != NULL) {
          BaudRate baudrate = getBaudRateBySpeed(speed);
          this->baudRateCallback(baudrate);
        }
        return true;
      }
      return false;
    }

    void sendMsg(Frame * frame) {
      CAN.sendMsgBuf(frame->canId(), frame->isExtended(), frame->dataSize(), frame->data());
    }
};
