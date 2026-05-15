#include <Arduino.h>
#include <SoftwareSerial.h>

#define KEYCOUNT 64
#define STOPCOUNT 3
#define STOPSHIFTS 5

#define MIDI_IN PD2
#define MIDI_OUT PD3

#define KEY_CHANNEL 1
#define STOP_CHANNEL 0

#define MUX0 PD4
#define MUX1 PD5
#define MUX2 PD6
#define MUXE PD7

#define KEY0 PB0
#define KEY1 PB1
#define KEY2 PB2
#define KEY3 PB3
#define KEY4 PB4
#define KEY5 PC0
#define KEY6 PC1
#define KEY7 PC2

SoftwareSerial midi(MIDI_IN, MIDI_OUT);

uint8_t keystate[64];
uint8_t stopstate[STOPCOUNT][STOPSHIFTS];

void setup() {
  for (uint8_t i=0; i<KEYCOUNT; i++) {
    keystate[i] = 0;
  }
  for (uint8_t i=0; i<STOPCOUNT; i++) {
    for (uint8_t j=0; j<STOPSHIFTS; j++) {
      stopstate[i][j] = 0;
    }
  }
  midi.begin(31250);
}

void loop() {
  // scan keyboard
  // scan stops
}
