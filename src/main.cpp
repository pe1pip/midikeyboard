/*

Copyright (c) 2026 Remco Post

This program is free software: you can redistribute it and/or modify it under the terms of the
GNU Affero General Public License as published by the Free Software Foundation, either version
3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License along with this program.
If not, see <https://www.gnu.org/licenses/>
*/

#include <Arduino.h>
#include <SoftwareSerial.h>

#define KEYCOUNT 64
#define STOPCOUNT 8

#define MIDI_IN 8
#define MIDI_OUT 9
#define MIDI_BAUDRATE 31250

#define KEY_CHANNEL 1
#define STOP_CHANNEL 0

#define KEY_OFF 0x80
#define KEY_ON 0x90

#define DEBOUNCE 63

#define KEY_PINMODE OUTPUT
#define GROUPLINE_PINMODE INPUT_PULLUP

#define DEBUG

#define GROUPLINE7 2
#define KEY0 3
#define KEY1 4
#define KEY2 5
#define KEY3 6
#define KEY4 7

#define STOPLINE 13

#ifdef DEBUG
uint8_t keyLines[5] = {KEY0, KEY1, KEY2, KEY3, KEY4};
uint8_t groupLines[1] = {GROUPLINE7};
uint8_t stopLines[1] = {STOPLINE};

uint8_t led = 1;

#else
#define GROUPLINE6 13
#define GROUPLINE5 14
#define GROUPLINE4 15
#define GROUPLINE3 16
#define GROUPLINE2 17
#define GROUPLINE1 18
#define GROUPLINE0 19

#define KEY5 10
#define KEY6 11
#define KEY7 12

uint8_t keyLines[8] = {KEY0, KEY1, KEY2, KEY3, KEY4, KEY5, KEY6, KEY7};
uint8_t groupLines[8] = {GROUPLINE0, GROUPLINE1, GROUPLINE2, GROUPLINE3, GROUPLINE4, GROUPLINE5, GROUPLINE6, GROUPLINE7};

#endif

#define STOP_OFF 0
#define STOP_ON_0 1
#define STOP_ON_1 2
#define STOP_ON_2 3
#define STOP_ON_MIN1 -1
#define STOP_ON_MIN2 -2

#define KEY_BASE 36 // C2

SoftwareSerial midi(MIDI_IN, MIDI_OUT);

int8_t stopShifts[6] = {STOP_OFF, STOP_ON_0, STOP_ON_1, STOP_ON_MIN1, STOP_ON_2, STOP_ON_MIN2};
int8_t keyState[KEYCOUNT];
int8_t stopState[STOPCOUNT];
int8_t stopShift[STOPCOUNT];

void scanKeyboard();
void scanStops();

void setupKeylines();
void setupGroupLines();
void setupStopLines();
void setupStops(); 

uint8_t calcStop (uint8_t stop, int8_t shift);

void sendMidi (uint8_t channel, uint8_t keyNum, uint8_t val);

void setup() {
  setupKeylines();
  setupGroupLines();
  setupStopLines();
  midi.begin(MIDI_BAUDRATE);
  Serial.begin(9600);
  Serial.println("Keyboard initialized");
#ifdef DEBUG
  Serial.println("Debug mode enabled");
  setupStops();
#endif
}

void loop() {
  scanKeyboard();
  scanStops();
#ifdef DEBUG
  led = !led;
  digitalWrite(LED_BUILTIN, led);
#endif
}

void setupKeylines () {
  for (uint8_t i=0; i<sizeof(keyLines); i++) {
    pinMode(keyLines[i], KEY_PINMODE);
  }
  for (uint8_t i=0; i<STOPCOUNT; i++) {
    stopState[i] = 0;
    stopShift[i] = 0;
  }
}

void setupGroupLines () {
  for (uint8_t i=0; i<sizeof(groupLines); i++) {
    pinMode(groupLines[i], GROUPLINE_PINMODE);
  }
#ifdef DEBUG
  pinMode(LED_BUILTIN, OUTPUT);
#endif
}

void setupStopLines () {
  for (uint8_t i=0; i<sizeof(stopLines); i++) {
    pinMode(stopLines[i], INPUT_PULLUP);
  }
}

void setupStops () {
  sendMidi(STOP_CHANNEL, calcStop(0, stopShift[1]), KEY_ON);
  sendMidi(STOP_CHANNEL, calcStop(1, stopShift[1]), KEY_ON);
  sendMidi(STOP_CHANNEL, calcStop(2, stopShift[1]), KEY_ON);
}

void scanKeyboard () {
  for (uint8_t line = 0; line < sizeof(keyLines); line++) {
    digitalWrite(keyLines[line], LOW);
    for (uint8_t i=0; i<sizeof(groupLines); i++) {
      uint8_t keyNum = (i * 8 + line) + 57;
      uint8_t val = digitalRead(groupLines[i]); // 0 if the key is pressed, 1 if it's not
      int8_t old = keyState[keyNum];
      if (old == 0 && val == 1) { // if the key was not pressed and now is
        keyState[keyNum] = DEBOUNCE;
        sendMidi(KEY_CHANNEL, keyNum, KEY_OFF);
      }
      if (old == 1 && val == 0) { // if the key was pressed and now is not
        keyState[keyNum] = -DEBOUNCE;
        sendMidi(KEY_CHANNEL, keyNum, KEY_ON);
      }
      if (old < 0) { // if the key was just released, count up to 0
        keyState[keyNum]++;
      }
      if (old > 1) { // if the key was just pressed, count down to 1
        keyState[keyNum]--;
      }
    }
    digitalWrite(keyLines[line], HIGH);
  }
}

void scanStops () {
#ifdef DEBUG
  for (uint8_t line = 0; line < sizeof(keyLines); line++) {
    digitalWrite(keyLines[line], LOW);
    for (uint8_t i=0; i<sizeof(stopLines); i++) {
      uint8_t val = digitalRead(stopLines[i]);
      uint8_t old = stopState[i];
      if (val ^ old && val) { // if val is not the same as the old state and it's now on
        stopState[i] = val;
        if (stopShift[i] == 5) { // if the stop is at max shift, the stof goes off
          stopShift[i] = 0;
          sendMidi(STOP_CHANNEL, calcStop(i, 5), KEY_OFF);
        } else { // STOP moves to next shift, from OFF to ON_0, to ON_1, etc
          if (stopShift[i] != 0) {
            sendMidi(STOP_CHANNEL, calcStop(i, stopShift[i]), KEY_OFF);
          }
          stopShift[i]++;
          sendMidi(STOP_CHANNEL, calcStop(i, stopShift[i]), KEY_ON);
        }
      }
    }
    digitalWrite(keyLines[line], HIGH);
  }
#endif
}

void sendMidi (uint8_t channel, uint8_t keyNum, uint8_t val) {
  uint8_t command = val + channel;
  uint8_t velocity = 0x7F;
  uint8_t key = keyNum + KEY_BASE;
  midi.write(command);
  midi.write(velocity);
  midi.write(key);
  Serial.println("Sent MIDI message on channel " + String(channel) + " with command " + String(val, HEX) + " and key " + String(key, HEX));
}

uint8_t calcStop (uint8_t stop, int8_t shift) {
  return stop + 4 + stopShifts[shift];
}