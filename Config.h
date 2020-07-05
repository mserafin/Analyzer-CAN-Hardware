#ifndef CONFIG_H
#define CONFIG_H

struct UARTConfig {
  uint32_t baudrate           = 115200;
  uint8_t  maxTimeOut         = 10;
  uint16_t frameLength        = 15;
  uint16_t responseInterval   = 5000;
  uint16_t responseBufferSize = 50;
};

struct CANConfig {
  uint8_t  mode         = 0x01;
  uint16_t baudrate     = 0x10;
  uint8_t  protocol     = 0x01;
  bool     scanEnable   = true;
  uint16_t scanInterval = 500;
};

#endif
