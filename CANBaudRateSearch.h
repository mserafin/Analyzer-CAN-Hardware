#pragma once

typedef void (*CANBaudRateSearchCallback)(BaudRate baudRate);

class CANBaudRateSearch
{
  private:
    struct Store {
      bool isSearching = false;
      uint16_t *intervalMillis;
      uint32_t lastReadMillis = 0L;
    } story;

    CANBaudRateIterator* iterator;
    CANBaudRateSearchCallback baudRateCallback;

  public:
    CANBaudRateSearch(CANBaudRateIterator* iterator, uint16_t* interval) {
      this->iterator = iterator;
      this->story.intervalMillis = interval;
    }

    void begin(CANBaudRateSearchCallback baudRateCallback) {
      this->baudRateCallback = baudRateCallback;
    }

    void enable() {
      iterator->rewind();
      story.isSearching = true;
    }

    void disable() {
      story.isSearching = false;
    }

    bool isEnable() {
      return story.isSearching;
    }

    void refresh() {
      if (story.isSearching && DateUtils::isDelaying(story.lastReadMillis, *story.intervalMillis)) {
        BaudRate *baudRate = iterator->next();
        if (baudRate) {
          baudRateCallback(*baudRate);
          story.lastReadMillis = DateUtils::now();
        } else {
          story.isSearching = false;
        }

        delete baudRate;
      }
    }

    ~CANBaudRateSearch() {
      delete iterator;
    }
};
