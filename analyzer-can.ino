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

ConnUART *serial    = new ConnUART(&configUART); //request
ConnCAN  *sniffer   = new ConnCAN();
Response *response = new Response(&configUART.intervalResponse);
BaudRateSearch *searchBaudRate = new BaudRateSearch(new BaudRateIterator(), &configCAN.intervalScan);

void setup() {
  serial->begin(onConfigChange, onDataSending);
  sniffer->begin(onDataReceived);
  response->begin(onDataResponse);
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
  response->refresh();
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
  if (!checkAnalyzerMode(configCAN.mode, Mode::TX))
  {
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

void onDataReceived(byte *data, byte dataSize)
{
  if (searchBaudRate->isEnable()) {
    searchBaudRate->disable();
  }

  if (checkAnalyzerMode(configCAN.mode, Mode::RX))
  {
    //    Serial.println("received: ");
    //    for (byte j = 0; j < dataSize; j++)
    //    {
    //      Serial.print(data[j]);
    //      Serial.print(",");
    //    }
    //    Serial.flush();

    response->append(data, dataSize);
  }
}

void onDataResponse(Message *buffer, int_res_index bufferSize) {
  Serial.println("sending: ");

  for (byte i = 0; i < bufferSize; i++)
  {
    Message m = buffer[i];
    //    Serial.write(m.data, m.dataSize);
    for (byte j = 0; j < m.dataSize; j++)
    {
      Serial.print(m.data[j]);
      Serial.print(",");
    }
    Serial.flush();
  }
}

void onBaudRateChange(BaudRate baudrate)
{
  if (baudrate.speed > 0) {
    configCAN.speed = baudrate.speed;
    sniffer->setBaudRate(baudrate.speed);
  }
  Serial.println(baudrate.name);
}

bool checkAnalyzerMode(byte pattern, Mode mode) {
  return (pattern & mode) == mode;
}
