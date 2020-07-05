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
#include "Response.h"

UARTConfig configUART;
CANConfig configCAN;

ConnUART* serial    = new ConnUART(&configUART); //request
ConnCAN* sniffer   = new ConnCAN();
Response* response = new Response(&configUART);
BaudRateSearch* searchBaudRate = new BaudRateSearch(new BaudRateIterator(), &configCAN.scanInterval);

void setup() {
  serial->begin(onConfigChange, onDataSending);
  sniffer->begin(onDataReceived);
  response->begin(onDataResponse);
  searchBaudRate->begin(onBaudRateChange);

  if (configCAN.scanEnable) {
    searchBaudRate->enable();
  } else {
    sniffer->setBaudRate(configCAN.baudrate);
  }
}

void loop() {
  serial->watching();
  sniffer->refresh();
  response->refresh();
  searchBaudRate->refresh();
}

void onConfigChange(const FrameType type, const uint32_t value)
{
  switch (type) {
    case CONFIG_UART: {
        configUART.baudrate = value;
        serial->setBaudRate(configUART.baudrate);
        Serial.print("CONFIG_UART: ");
        Serial.println(configUART.baudrate);
      }
      break;
    case CONFIG_CAN:
      configCAN.mode     = (value >> 28) & 0xF;
      configCAN.protocol = (value >> 24) & 0xF;
      configCAN.baudrate    = value & 0xFFFF;

      if (searchBaudRate->isEnable()) {
        searchBaudRate->disable();
      }
      sniffer->setBaudRate(configCAN.baudrate);

      Serial.print("CONFIG_CAN: mode=");
      Serial.print(configCAN.mode);
      Serial.print(", protocol=");
      Serial.print(configCAN.protocol);
      Serial.print(", speed=");
      Serial.println(configCAN.baudrate);
      break;
    case SYNC_TIME:
      Serial.print("SYNC_TIME: ");
      Serial.println(value);
      break;
  }
}

void onDataSending(const Frame* frame)
{
  if (!checkAnalyzerMode(configCAN.mode, Mode::TX)) {
    return;
  }

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

void onDataReceived(const byte* frame, const byte frameSize)
{
  if (searchBaudRate->isEnable()) {
    searchBaudRate->disable();
  }

  if (checkAnalyzerMode(configCAN.mode, Mode::RX)) {
    response->append(frame, frameSize);
  }
}

void onDataResponse(const byte* buffer, const uint16_t bufferSize) {
  //  Serial.write(buffer, bufferSize);

  Serial.print("sending: ");
  Serial.print(response->lastCountFrames());
  Serial.print(" size: ");
  Serial.println(bufferSize);

  for (byte i = 0; i < bufferSize; i++)
  {
    Serial.print(buffer[i]);
    Serial.print(",");
  }
}

void onBaudRateChange(BaudRate baudrate)
{
  if (baudrate.speed > 0) {
    configCAN.baudrate = baudrate.speed;
    sniffer->setBaudRate(baudrate.speed);
  }
  Serial.println(baudrate.name);
}

bool checkAnalyzerMode(byte pattern, Mode mode) {
  return (pattern & mode) == mode;
}
