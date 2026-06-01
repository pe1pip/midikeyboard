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
#include <stops.h>
#include <midi.h>

uint8_t stopLines[3] = {STOP0, STOP1, STOP2};
uint8_t stopLeds[3] = {STOP0LED, STOP1LED, STOP2LED};
int8_t stopShifts[6] = {STOP_OFF, STOP_ON_0, STOP_ON_1, STOP_ON_MIN1, STOP_ON_2, STOP_ON_MIN2};
int8_t stopState[STOPCOUNT] = { 1, 1, 1 }; // we start with all stops on, so the organ makes sound when powered on
int8_t stopShift[STOPCOUNT] = { 0 }; // we start with all stops in the normal position, so shift 0
boolean stopChanged[STOPCOUNT] = { false }; // whether the state of the stop has changed since the last scan, so we know when to send MIDI messages

// a list of Interrupt Service Routines for easy setup of the stop line interrupts
void (*isrFunctions[STOPCOUNT * 3])() = {stop0ISR, stop1ISR, stop2ISR, stop3ISR, stop4ISR, stop5ISR, stop6ISR, stop7ISR, stop8ISR};

void setupStops () {
  for (uint8_t i=0; i<STOPCOUNT; i++) {
    stopState[i] = 0;
    stopShift[i] = 0;
    stopChanged[i] = false;
  }
  sendMidi(STOP_CHANNEL, calcStop(0, stopShift[1]), KEY_ON);
  sendMidi(STOP_CHANNEL, calcStop(1, stopShift[1]), KEY_ON);
  sendMidi(STOP_CHANNEL, calcStop(2, stopShift[1]), KEY_ON);
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