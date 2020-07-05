#pragma once

typedef void (*ResponseSendDataCallback)(const byte* buffer, const uint16_t bufferSize);

class Response
{
  private:
    UARTConfig* uart;
    ResponseSendDataCallback sendDataCallback;

    uint16_t lastCount = 0;
    uint32_t lastSend  = 0L;

    struct Buffer {
      byte* data = nullptr;
      uint16_t cursor = 0;
      void clear() {
        cursor = 0;
      }
      ~Buffer() {
        if (data) {
          delete[] data;
          data = nullptr;
        }
      }
    } buffer;

  public:
    Response(UARTConfig* uart) {
      this->uart = uart;
    }

    void begin(ResponseSendDataCallback sendDataCallback) {
      buffer.data = new byte[uart->responseBufferSize * FRAME_LENGTH];
      this->sendDataCallback = sendDataCallback;
    }

    void refresh() {
      if (currentCountFrames() > 0 && (!hasNext() || DateUtils::isDelaying(lastSend, uart->responseInterval))) {
        lastCount = currentCountFrames();
        sendDataCallback(buffer.data, buffer.cursor);
        buffer.clear();
        lastSend = DateUtils::now();
      }
    }

    void append(const byte* frame, const byte frameSize) {
      if (hasNext()) {
        for (byte i = 0; i < frameSize; i++) {
          *(buffer.data + (buffer.cursor++)) = *(frame++);
        }
      }
    }

    bool hasNext() {
      return currentCountFrames() < uart->responseBufferSize;
    }

    uint16_t currentCountFrames() {
      return (buffer.cursor / FRAME_LENGTH);
    }

    uint16_t lastCountFrames() {
      return lastCount;
    }

    ~Response() {
      buffer.~Buffer();
    }
};
