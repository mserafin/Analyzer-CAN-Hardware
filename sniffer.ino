#include "Config.h"
#include "Convert.h"
#include "ByteUtils.h"
#include "ConnUart.h"
#include "ConnCan.h"

UARTConfig uart;
CANConfig configCan;

ConnUART *serial = new ConnUART(&uart);
ConnCAN  *mcp    = new ConnCAN(&configCan);

void setup() {
  serial->begin(onConfigChange, onDataSending);
  mcp->begin(onDataReceived, onBaudRateChange);
}

void loop() {
  serial->watching();
  mcp->watching();
}

void onConfigChange(FrameType type, uint32_t value)
{
  switch (type) {
    case CONFIG_UART: {
        uart.baudrate = value;
        //serial->setBaudrate(uart.baudrate);
        Serial.print("CONFIG_UART: ");
        Serial.println(uart.baudrate);
      }
      break;
    case CONFIG_CAN:
      configCan.mode     = (value >> 28) & 0xF;
      configCan.protocol = (value >> 24) & 0xF;
      configCan.speed    = value & 0xFFFF;

      mcp->setSpeed(configCan.speed);

      Serial.print("CONFIG_CAN: mode=");
      Serial.print(configCan.mode);
      Serial.print(", protocol=");
      Serial.print(configCan.protocol);
      Serial.print(", speed=");
      Serial.println(configCan.speed);
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

  Serial.print("DataSize: ");
  Serial.println(frame->dataSize());

  Serial.print("IsExtended: ");
  Serial.println(frame->isExtended());

  Serial.print("Data send: ");
  for (byte i = 0, l = frame->dataSize(); i < l; i++)
  {
    Serial.print(frame->data(i));
    Serial.print(",");
  }
  Serial.println();

  //mcp->sendMsg(frame);
  //delete frame;
}

void onDataReceived(byte *data, byte dataSize) {
  Serial.print("Data received: ");
  for (byte i = 0; i < dataSize; i++)
  {
    Serial.print(data[i]);
    Serial.print(",");
  }
  Serial.println();
}

void onBaudRateChange(BaudRate baudrate) {
  configCan.speed = baudrate.speed;

  Serial.print("CAN BUS Shield init ok: ");
  Serial.println(baudrate.name);
}
