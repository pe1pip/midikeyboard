#include <Arduino.h>
#include <SoftwareSerial.h>

#define KEYCOUNT 64
#define STOPCOUNT 8

#define MIDI_IN PD2
#define MIDI_OUT PD3
#define MIDI_BAUDRATE 31250

#define KEY_CHANNEL 1
#define STOP_CHANNEL 0

#define KEY_OFF 0x80
#define KEY_ON 0x90

#define MUX_OUT_0 PD4
#define MUX_OUT_1 PD5
#define MUX_OUT_2 PD6
#define MUX_OUT_0E PD7
#define MUX_OUT_1E PC3

#define KEY_PINMODE INPUT
#define KEY0 PB0
#define KEY1 PB1
#define KEY2 PB2
#define KEY3 PB3
#define KEY4 PB4
#define KEY5 PC0
#define KEY6 PC1
#define KEY7 PC2

#define STOP_OFF 0
#define STOP_ON_0 1
#define STOP_ON_1 2
#define STOP_ON_2 3
#define STOP_ON_MIN1 -1
#define STOP_ON_MIN2 -2

#define KEY_BASE 36 // C2

SoftwareSerial midi(MIDI_IN, MIDI_OUT);

uint8_t senseline[8] = {KEY0, KEY1, KEY2, KEY3, KEY4, KEY5, KEY6, KEY7};
int8_t stopShifts[6] = {STOP_OFF, STOP_ON_0, STOP_ON_1, STOP_ON_MIN1, STOP_ON_2, STOP_ON_MIN2};
uint8_t keystate[KEYCOUNT];
uint8_t stopstate[STOPCOUNT];
int8_t stopShift[STOPCOUNT];

void scanKeyboard ();
void scanStops ();

void setupOutputs ();
void setupInputs();

uint8_t calcStop (uint8_t stop, int8_t shift);

void sendMidi (uint8_t channel, uint8_t keyNum, uint8_t val);

void setup() {
  setupInputs();
  setupOutputs();
  midi.begin(MIDI_BAUDRATE);
}

void loop() {
  scanKeyboard();
  scanStops();
}

void setupInputs () {
  pinMode(KEY0, KEY_PINMODE);
  pinMode(KEY1, KEY_PINMODE);
  pinMode(KEY2, KEY_PINMODE);
  pinMode(KEY3, KEY_PINMODE);
  pinMode(KEY4, KEY_PINMODE);
  pinMode(KEY5, KEY_PINMODE);
  pinMode(KEY6, KEY_PINMODE);
  pinMode(KEY7, KEY_PINMODE);
}

void setupOutputs () {
  pinMode(MUX_OUT_0, OUTPUT);
  pinMode(MUX_OUT_1, OUTPUT);
  pinMode(MUX_OUT_2, OUTPUT);
  pinMode(MUX_OUT_0E, OUTPUT);
  pinMode(MUX_OUT_1E, OUTPUT);
}

void scanKeyboard () {
  for (uint8_t bit_2 = 0; bit_2 < 2; bit_2++ ) {
    digitalWrite(MUX_OUT_2, bit_2);
    for (uint8_t bit_1 = 0; bit_1 < 2; bit_1++ ) {
      digitalWrite(MUX_OUT_1, bit_1);
      for (uint8_t bit_0 = 0; bit_0 < 2; bit_0++ ) {
        digitalWrite(MUX_OUT_0, bit_0);
        digitalWrite(MUX_OUT_0E, HIGH);
        uint8_t base = (bit_2 * 4 + bit_1 * 2 + bit_0) * 8;
        for (uint8_t line = 0; line < 8; line++) {
          uint8_t keyNum = base + line;
          uint8_t val = digitalRead(senseline[line]);
          uint8_t old = keystate[keyNum];
          if (val ^ old) { // if val is not the same as the old state
            keystate[keyNum] = val;
            sendMidi(KEY_CHANNEL, keyNum, val);
          }
        }
        digitalWrite(MUX_OUT_0E, LOW);
      }
    }
  }
}

void scanStops () {
  // for now no MUX, just 
  digitalWrite(MUX_OUT_1E, HIGH);
  for (uint8_t line = 0; line < 8; line++) {
    uint8_t val = digitalRead(senseline[line]);  
    uint8_t old = stopstate[line];
    if (val ^ old && val) { // if val is not the same as the old state and it's now on
      stopstate[line] = val;
      if (stopShift[line] == 5) { // if the stop is at max shift, the stof goes off
        stopShift[line] = 0;
        sendMidi(STOP_CHANNEL, calcStop(line, 5), KEY_OFF);
      } else { // STOP moves to next shift, from OFF to ON_0, to ON_1, etc
        if (stopShift[line] != 0) {
          sendMidi(STOP_CHANNEL, calcStop(line, stopShift[line]), KEY_OFF);
        }
        stopShift[line]++;
        sendMidi(STOP_CHANNEL, calcStop(line, stopShift[line]), KEY_ON);
      }
    }
    digitalWrite(MUX_OUT_1E, LOW);
  }
}

void sendMidi (uint8_t channel, uint8_t keyNum, uint8_t val) {
  uint8_t command = (val * 0x10) + channel;
  uint8_t velocity = 0x7F;
  uint8_t key = keyNum + KEY_BASE;
  midi.write(command);
  midi.write(velocity);
  midi.write(key);
}

uint8_t calcStop (uint8_t stop, int8_t shift) {
  return stop + 4 + stopShifts[shift];
}