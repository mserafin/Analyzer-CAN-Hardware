#pragma once

typedef void (*SearchBaudRateCallback)(BaudRate baudRate);

class BaudRateSearch
{
  private:
    struct Store {
      bool isSearching = false;
      uint16_t *delayMillis;
      uint32_t lastReadMillis = 0L;
    } story;

    BaudRateIterator *iterator;
    SearchBaudRateCallback baudRateCallback;

  public:
    BaudRateSearch(BaudRateIterator *iterator, uint16_t *delay) {
      this->iterator = iterator;
      this->story.delayMillis = delay;
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
      if (story.isSearching && DateUtils::isDelaying(story.lastReadMillis, *story.delayMillis)) {
        BaudRate *baudRate = iterator->next();
        if (baudRate != NULL) {
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
