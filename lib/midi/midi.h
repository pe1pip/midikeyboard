namespace midi {
  uint8_t calcStop(uint8_t stop, int8_t shift);
  void send(uint8_t channel, uint8_t keyNum, uint8_t midiCommand);
}