#pragma once

typedef void (*SearchBaudRateCallback)(BaudRate baudRate);

class BaudRateSearch
{
  private:
    struct Store {
      bool isSearching = false;
      uint16_t *intervalMillis;
      uint32_t lastReadMillis = 0L;
    } story;

    BaudRateIterator *iterator;
    SearchBaudRateCallback baudRateCallback;

  public:
    BaudRateSearch(BaudRateIterator *iterator, uint16_t *interval) {
      this->iterator = iterator;
      this->story.intervalMillis = interval;
    }

    void begin(SearchBaudRateCallback baudRateCallback)
    {
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

    ~BaudRateSearch() {
      delete iterator;
    }
};
