#pragma once

const uint16_t BAUD_RATE_SIZE = 0x11;

struct BaudRate {
  uint16_t speed;
  String name;
};

class BaudRateIterator
{
  private:
    struct BaudRate baudRate[BAUD_RATE_SIZE] =
    {
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
      {CAN_5KBPS,    "CAN 5"},
      {0, "UNKNOWN"}
    };

    uint16_t index = 0;

  public:
    BaudRate *next() {
      if (!hasNext()) {
        return nullptr;
      }
      return &baudRate[index++];
    }

    bool hasNext() {
      return index < BAUD_RATE_SIZE;
    }

    void rewind() {
      index = 0;
    }

    BaudRate *current() {
      return &baudRate[index];
    }
};
