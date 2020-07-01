#pragma once

class DateUtils
{
  public:
    static uint8_t isDelaying(uint32_t timeInMillis, uint16_t interval) {
      return (now() - timeInMillis) > interval;
    }

    static uint32_t now() {
      return millis();
    }
};
