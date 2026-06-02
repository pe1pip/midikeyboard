#include <Arduino.h>
#include <generic.h>

// MIDI related constants and variables
#define MIDI_IN 8
#define MIDI_OUT 9
#define MIDI_BAUDRATE 31250
#define midi Serial3

#define KEY_CHANNEL 1
#define STOP_CHANNEL 0

#define KEY_OFF 0x80
#define KEY_ON 0x90

#define KEY_BASE 36 // C2

// MIDI related functions
uint8_t calcStop (uint8_t stop, int8_t shift);
void sendMidi (uint8_t channel, uint8_t keyNum, uint8_t midiCommand);