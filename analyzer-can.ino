#include <SPI.h>
#include "mcp_can.h"
#include "Config.h"
#include "Convert.h"
#include "ByteUtils.h"
#include "DateUtils.h"
#include "Frame.h"
#include "ConnUart.h"
#include "BaudRateIterator.h" //CANBaudRateIterator
#include "BaudRateSearch.h"   //CANBaudRateSearch
#include "ConnCan.h"          //CANSniffer

UARTConfig configUART;
CANConfig configCAN;

ConnUART *serial    = new ConnUART(&configUART);
ConnCAN  *sniffer   = new ConnCAN();
BaudRateSearch *searchBaudRate = new BaudRateSearch(new BaudRateIterator(), &configCAN.delayScan);

void setup() {
  serial->begin(onConfigChange, onDataSending);
  sniffer->begin(onDataReceived);
  searchBaudRate->begin(onBaudRateChange);

  if (configCAN.autoScan) {
    searchBaudRate->enable();
  } else {
    sniffer->setBaudRate(configCAN.speed);
  }
}

void loop() {
  serial->watching();
  sniffer->refresh();
  searchBaudRate->refresh();
}

void onConfigChange(FrameType type, uint32_t value)
{
  switch (type) {
    case CONFIG_UART: {
        configUART.baudrate = value;
        //serial->setBaudRate(configUART.baudrate);
        Serial.print("CONFIG_UART: ");
        Serial.println(configUART.baudrate);
      }
      break;
    case CONFIG_CAN:
      configCAN.mode     = (value >> 28) & 0xF;
      configCAN.protocol = (value >> 24) & 0xF;
      configCAN.speed    = value & 0xFFFF;

      if (searchBaudRate->isEnable()) {
        searchBaudRate->disable();
      }
      sniffer->setBaudRate(configCAN.speed);

      Serial.print("CONFIG_CAN: mode=");
      Serial.print(configCAN.mode);
      Serial.print(", protocol=");
      Serial.print(configCAN.protocol);
      Serial.print(", speed=");
      Serial.println(configCAN.speed);
      break;
    case SYNC_TIME:
      Serial.print("SYNC_TIME: ");
      Serial.println(value);
      break;
  }
}

void onDataSending(Frame *frame)
{
  Serial.print("CanId: ");
  Serial.println(frame->canId());
  //
  //  Serial.print("DataSize: ");
  //  Serial.println(frame.dataSize());
  //
  //  Serial.print("IsExtended: ");
  //  Serial.println(frame.isExtended());
  //
  //  Serial.print("Data send: ");
  //  for (byte i = 0, l = frame.dataSize(); i < l; i++)
  //  {
  //    Serial.print(frame.data(i));
  //    Serial.print(",");
  //  }
  //  Serial.println();

  sniffer->sendMsg(frame);
}

void onDataReceived(byte *data, byte dataSize)
{
  if (searchBaudRate->isEnable()) {
    searchBaudRate->disable();
  }

  Serial.print("Data received: ");
  for (byte i = 0; i < dataSize; i++)
  {
    Serial.print(data[i]);
    Serial.print(",");
  }
  Serial.println();
}

void onBaudRateChange(BaudRate baudrate)
{
  if (baudrate.speed > 0) {
    configCAN.speed = baudrate.speed;
    sniffer->setBaudRate(baudrate.speed);
  }
  Serial.println(baudrate.name);
}
