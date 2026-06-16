#define MIDI_OK 0
#define MIDI_NOTCONN 1
#define MIDI_ERROR 2
#define MIDI_UNAVAIL -1

namespace wifiMidi {
  void init();
  int send(uint8_t channel, uint8_t command, uint8_t keyNum);
  void blinkLed();
  void scan();
}

