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

#define DEBUG

// general constants
#define DEBOUNCE 63

// MIDI related constants and variables
#define MIDI_IN 8
#define MIDI_OUT 9
#define MIDI_BAUDRATE 31250
#define midi Serial4 // RX3/TX3

#define KEY_CHANNEL 1
#define STOP_CHANNEL 0

#define KEY_OFF 0x80
#define KEY_ON 0x90

// keyboard related constants and variables
#define KEY_PINMODE OUTPUT
#define GROUPLINE_PINMODE INPUT_PULLUP

#define KEY_BASE 36 // C2
#define KEYCOUNT 64
#define GROUPLINE0 53
#define GROUPLINE1 52
#define GROUPLINE2 51
#define GROUPLINE3 50
#define GROUPLINE4 49 
#define GROUPLINE5 48
#define GROUPLINE6 47
#define GROUPLINE7 46
#define KEY0 41
#define KEY1 44
#define KEY2 42
#define KEY3 45
#define KEY4 43
#define KEY5 39
#define KEY6 38
#define KEY7 40

uint8_t keyLines[8] = {KEY0, KEY1, KEY2, KEY3, KEY4, KEY5, KEY6, KEY7};
uint8_t groupLines[8] = {GROUPLINE0, GROUPLINE1, GROUPLINE2, GROUPLINE3, GROUPLINE4, GROUPLINE5, GROUPLINE6, GROUPLINE7};
int8_t keyState[KEYCOUNT];

// stop related constants and variables
#define STOPCOUNT 3

#define STOP0 2
#define STOP1 4
#define STOP2 6

#define STOP0LED 3
#define STOP1LED 5
#define STOP2LED 7

#define STOP_OFF 0
#define STOP_ON_0 1
#define STOP_ON_1 2
#define STOP_ON_2 3
#define STOP_ON_MIN1 -1
#define STOP_ON_MIN2 -2

uint8_t stopLines[3] = {STOP0, STOP1, STOP2};
uint8_t stopLeds[3] = {STOP0LED, STOP1LED, STOP2LED};
int8_t stopShifts[6] = {STOP_OFF, STOP_ON_0, STOP_ON_1, STOP_ON_MIN1, STOP_ON_2, STOP_ON_MIN2};
int8_t stopState[STOPCOUNT] = { 1, 1, 1 }; // we start with all stops on, so the organ makes sound when powered on
int8_t stopShift[STOPCOUNT] = { 0 }; // we start with all stops in the normal position, so shift 0
boolean stopChanged[STOPCOUNT] = { false }; // whether the state of the stop has changed since the last scan, so we know when to send MIDI messages

// calcant related constants and variables
#define CALCANT 15
#define CALCANT_LED 17

int8_t calcant;

// setup function declarations
void setupKeylines();
void setupGroupLines();
void setupStopLines();
void setupStops();
void setupCalcant();

// scan loop declarations
void scanKeyboard();
void scanStops();
void blinkCalcant();
void scanCalcant();

// function declarations for the ISRs for the stop lines
void stop0ISR();
void stop1ISR();
void stop2ISR();
void stop3ISR();
void stop4ISR();
void stop5ISR();
void stop6ISR();
void stop7ISR();
void stop8ISR();
void stopISR (int stopNum, int stopShift);

// MIDI related functions
uint8_t calcStop (uint8_t stop, int8_t shift);
void sendMidi (uint8_t channel, uint8_t keyNum, uint8_t val);

// a list of Interrupt Service Routines for easy setup of the stop line interrupts
void (*isrFunctions[STOPCOUNT * 3])() = {stop0ISR, stop1ISR, stop2ISR, stop3ISR, stop4ISR, stop5ISR, stop6ISR, stop7ISR, stop8ISR};

void setup() {
  setupKeylines();
  setupGroupLines();
  setupStopLines();
  setupCalcant();
  Serial.begin(115200);
  while(!Serial); // wait for serial to be ready
  delay(250); // wait for the serial monitor to open
  Serial.println("Keyboard initialized");
  midi.begin(MIDI_BAUDRATE, SERIAL_8N1);
#ifdef DEBUG
  Serial.println("Debug mode enabled");
  setupStops();
#endif
}

void loop() {
  // scanKeyboard();
  scanStops();
  scanCalcant();
  blinkCalcant();
}

/**
 * Sets up the key lines.
 */
void setupKeylines () {
  for (uint8_t i=0; i<sizeof(keyLines); i++) {
    pinMode(keyLines[i], KEY_PINMODE);
  }
  for (uint8_t i=0; i<STOPCOUNT; i++) {
    stopState[i] = 0;
    stopShift[i] = 0;
    stopChanged[i] = false;
  }
}

/**
 * Sets up the group lines.
 */
void setupGroupLines () {
  for (uint8_t i=0; i<sizeof(groupLines); i++) {
    pinMode(groupLines[i], GROUPLINE_PINMODE);
  }
}

/**
 * Sets up the stop lines and their associated LEDs.
 */
void setupStopLines () {
  for (uint8_t i=0; i<sizeof(stopLines); i++) {
    pinMode(stopLines[i], INPUT_PULLUP);
    pinMode(stopLeds[i], OUTPUT);
    attachInterrupt(digitalPinToInterrupt(stopLines[i]), isrFunctions[1 + (i * 3)], FALLING); // we want to know when the key is being pressed
  }
}

/**
 * Sets up the calcant switch and its associated LED.
 */
void setupCalcant () {
  pinMode(CALCANT, INPUT_PULLUP);
  pinMode(CALCANT_LED, OUTPUT);
  digitalWrite(CALCANT_LED, LOW);
}

void setupStops () {
  sendMidi(STOP_CHANNEL, calcStop(0, stopShift[1]), KEY_ON);
  sendMidi(STOP_CHANNEL, calcStop(1, stopShift[1]), KEY_ON);
  sendMidi(STOP_CHANNEL, calcStop(2, stopShift[1]), KEY_ON);
}

/**
 * Scans the keyboard and updates the state of each key.
 */
void scanKeyboard () {
  for (uint8_t line = 0; line < sizeof(keyLines); line++) {
    digitalWrite(keyLines[line], LOW);
    for (uint8_t i=0; i<sizeof(groupLines); i++) {
      uint8_t keyNum = i * 8 + line;
      int8_t old = keyState[keyNum];
      if (old < 0) { // if the key was just released, count up to 0
        keyState[keyNum]++;
        return;
      }
      if (old > 1) { // if the key was just pressed, count down to 1
        keyState[keyNum]--;
        return;
      }
      uint8_t val = digitalRead(groupLines[i]); // 0 if the key is pressed, 1 if it's not

      if (old == 0 && val == 1) { // if the key was not pressed and now is
        keyState[keyNum] = DEBOUNCE;
        sendMidi(KEY_CHANNEL, keyNum + KEY_BASE, KEY_OFF);
        return;
      }
      if (old == 1 && val == 0) { // if the key was pressed and now is not
        keyState[keyNum] = -DEBOUNCE;
        sendMidi(KEY_CHANNEL, keyNum + KEY_BASE, KEY_ON);
        return;
      }
    }
    digitalWrite(keyLines[line], HIGH);
  }
}

/**
 * Scans the stop lines and updates their state.
 */
void scanStops () {
  for (uint8_t i=0; i<STOPCOUNT; i++) {
    if (stopChanged[i]) {
      #ifdef DEBUG
        Serial.println("Stop " + String(i) + " changed to " + String(stopState[i]) + " with shift " + String(stopShift[i]));
      #endif
      stopChanged[i] = false;
      if (stopState[i] > 0) { // if the stop is on
        digitalWrite(stopLeds[i], HIGH);
        sendMidi(STOP_CHANNEL, calcStop(i, stopShift[i]), KEY_ON);
      } else { // if the stop is off
        digitalWrite(stopLeds[i], LOW);
        sendMidi(STOP_CHANNEL, calcStop(i, stopShift[i]), KEY_OFF);
      }
    }
  }
}

/**
 * calculates the MIDI key number for a given stop and shift
 * @param stop The stop number (0-127)
 * @param shift The shift value (-2 to 2)
 * @return The MIDI key number
 */
uint8_t calcStop (uint8_t stop, int8_t shift) {
  return stop + 4 + stopShifts[shift];
}

/**
 * If the blower is off we blink the led to indicate that the organ is not making sound,
 * otherwise we keep it on to indicate that the organ is making sound.
 */
void blinkCalcant () {
  if (calcant < 1) {
    if (millis() % 1000 < 500) {
      digitalWrite(CALCANT_LED, HIGH);
    } else {
      digitalWrite(CALCANT_LED, LOW);
    }
  } else {
    digitalWrite(CALCANT_LED, HIGH);
  }
}

/**
 * Scans the calcant switch and updates its state.
 */
void scanCalcant () {
  // debounce timers
  if (calcant > 1) {
    calcant--;
    return;
  }
  if (calcant < 0) {
    calcant++;
    return;
  }
  // calcant current state is now either 1 or 0, so we can read the state of calcant
  // switch and compare it to the current state to see if it has changed
  uint8_t val = digitalRead(CALCANT);
  if (calcant == 0 && val > 0) {
    calcant = DEBOUNCE; // if the blower was off and now is on, start debouncing
    sendMidi(STOP_CHANNEL, 0, KEY_ON);
    return;
  }
  if (calcant == 1 && val == 0) {
    calcant = -DEBOUNCE; // if the blower was on and now is off, start debouncing
    sendMidi(STOP_CHANNEL, 0, KEY_OFF);
    return;
  }
}

/**
 * Sends a MIDI message on the specified channel with the given key number and value.
 * @param channel The MIDI channel to send the message on (0-15).
 * @param keyNum The MIDI key number to send (0-127).
 * @param val The MIDI command to send (e.g. KEY_ON or KEY_OFF).
 */
void sendMidi (uint8_t channel, uint8_t keyNum, uint8_t val) {
  uint8_t command = val + channel;
  uint8_t velocity = 0x7F;
  uint8_t key = keyNum + KEY_BASE;
  midi.write(command);
  midi.write(velocity);
  midi .write(key);
  #ifdef DEBUG
  Serial.println("Sent MIDI message on channel " + String(channel) + " with command " + String(val, HEX) + " and key " + String(key, HEX));
  #endif
}

// 
void stop0ISR () {
  stopISR(0, -1);
}
void stop1ISR () {
  stopISR(0, 0);
}
void stop2ISR () {
  stopISR(0, 1);
}
void stop3ISR () {
  stopISR(1, -1);
}
void stop4ISR () {
  stopISR(1, 0);
}
void stop5ISR () {
  stopISR(1, 1);
}
void stop6ISR () {
  stopISR(2, -1);
}
void stop7ISR () {
  stopISR(2, 0);
}
void stop8ISR () {
  stopISR(2, 1);
}

void stopISR (int stopNum, int stopShift) {
  stopState[stopNum] = !stopState[stopNum];
  stopChanged[stopNum] = true;
  if (stopState[stopNum]) { // if the stop is now on
    stopShifts[stopNum] = stopShift;
  } else { // if the stop is now off
    stopShifts[stopNum] = 0;
  }
  return;
}