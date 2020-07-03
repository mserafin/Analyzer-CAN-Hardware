#ifndef CONFIG_H
#define CONFIG_H

struct UARTConfig {
  uint32_t baudrate     = 115200;
  uint16_t intervalResponse = 1000;
  uint16_t maxFrameSize = 15;
  uint8_t  maxTimeOut    = 10;
};

struct CANConfig {
  uint8_t  mode      = 0x01;
  uint8_t  protocol  = 0x01;
  uint16_t speed     = 0x10;
  uint16_t intervalScan = 500;
  bool     autoScan  = true;
};

#endif
