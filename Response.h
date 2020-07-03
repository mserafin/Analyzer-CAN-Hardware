#pragma once

struct Message {
  byte *data;
  byte dataSize;
};

typedef uint16_t int_res_index;
typedef void (*ResponseDataCallback)(Message *buffer, int_res_index bufferSize);

const int_res_index RESPONSE_BUFFER_SIZE = 100;

class Response
{
  private:
    ResponseDataCallback dataCallback;

    Message *messages = new Message[RESPONSE_BUFFER_SIZE];
    int_res_index index = 0;

    struct Sender {
      uint16_t *intervalMillis;
      uint32_t lastReadMillis = 0L;
    } sender;

    void clearBuffer() {
      if (count() > 0) {
        //        for (int_res_index i = 0, l = count(); i < l; i++) {
        //          delete messages[i].data;
        //        }
        index = 0;
      }
    }

  public:
    Response(uint16_t *interval) {
      sender.intervalMillis = interval;
    }

    void begin(ResponseDataCallback dataCallback) {
      this->dataCallback = dataCallback;
      clearBuffer();
    }

    void refresh() {
      if (count() > 0 && (!hasNext() || DateUtils::isDelaying(sender.lastReadMillis, *sender.intervalMillis))) {
        dataCallback(messages, count());
        clearBuffer();
        sender.lastReadMillis = DateUtils::now();
      }
    }

    void append(byte *data, byte dataSize) {
      if (hasNext()) {
        messages[index++] = {data, dataSize};
      }
    }

    bool hasNext() {
      return index < RESPONSE_BUFFER_SIZE;
    }

    int_res_index count() {
      return index;
    }
};
