const byte SPI_CS_PIN   = 10;

const byte FRAME_LENGTH = 16;

#include <SPI.h>
#include "mcp_can.h"
#include "Config.h"
#include "Convert.h"
#include "ByteUtils.h"
#include "DateUtils.h"
#include "Frame.h"
#include "CANBaudRateIterator.h"
#include "CANBaudRateSearch.h"
#include "CANSniffer.h"
#include "Response.h"
#include "Request.h"

UARTConfig uart;
CANConfig can;

Request*  request   = new Request(&uart);
Response* response  = new Response(&uart);
CANSniffer* sniffer = new CANSniffer(SPI_CS_PIN);
CANBaudRateSearch* searchBaudRate = new CANBaudRateSearch(new CANBaudRateIterator(), &can.scanInterval);

void setup() {
  request->begin(onConfigChange, onDataSending);
  response->begin(onDataResponse);

  sniffer->begin(onDataReceived);
  searchBaudRate->begin(onBaudRateChange);

  if (can.scanEnable) {
    searchBaudRate->enable();
  } else {
    sniffer->setBaudRate(can.baudrate);
  }
}

void loop() {
  request->refresh();
  searchBaudRate->refresh();
  sniffer->refresh();
  response->refresh();
}

void onConfigChange(const FrameType type, const uint32_t value)
{
  switch (type) {
    case CONFIG_UART: {
        uart.baudrate = value;
        request->setBaudRate(uart.baudrate);
      }
      break;
    case CONFIG_CAN: {
        can.mode     = (value >> 28) & 0xF;
        can.protocol = (value >> 24) & 0xF;
        can.baudrate    = value & 0xFFFF;

        if (searchBaudRate->isEnable()) {
          searchBaudRate->disable();
        }

        bool isBaudRate = sniffer->setBaudRate(can.baudrate);

        if (isBaudRate && checkProtocol(can.protocol, Protocol::OBD)) {
          sniffer->enableFilterOBD();
        }
      }
      break;
    case SYNC_TIME:
      break;
  }
}

void onDataSending(const Frame* frame)
{
  if (!checkMode(can.mode, Mode::TX)) {
    return;
  }
  sniffer->sendMsg(frame);
}

void onDataReceived(const byte* frame, const byte frameSize)
{
  if (searchBaudRate->isEnable()) {
    searchBaudRate->disable();
  }

  if (checkMode(can.mode, Mode::RX)) {
    response->append(frame, frameSize);
  }
}

void onDataResponse(const byte* buffer, const uint16_t bufferSize) {
  Serial.write(buffer, bufferSize);
}

void onBaudRateChange(BaudRate baudrate)
{
  if (baudrate.speed > 0) {
    can.baudrate = baudrate.speed;
    sniffer->setBaudRate(baudrate.speed);
  }
}

bool checkMode(byte pattern, Mode mode) {
  return (pattern & mode) == mode;
}

bool checkProtocol(byte pattern, Protocol protocol) {
  return (pattern & protocol) == protocol;
}
