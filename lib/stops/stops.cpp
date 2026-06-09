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
#include <generic.h>
#include <stops.h>
#include <midi.h>

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

namespace stops {
  uint8_t stopLines[STOPCOUNT] = {STOP0, STOP1, STOP2};
  uint8_t stopLeds[STOPCOUNT] = {STOP0LED, STOP1LED, STOP2LED};
  int8_t stopShifts[6] = {STOP_OFF, STOP_ON_0, STOP_ON_1, STOP_ON_MIN1, STOP_ON_2, STOP_ON_MIN2};
  int8_t stopState[STOPCOUNT] = { 1, 1, 1 }; // we start with all stops on, so the organ makes sound when powered on
  int8_t stopDebounce[STOPCOUNT] = { 0 };
  int8_t stopShift[STOPCOUNT] = { 0 }; // we start with all stops in the normal position, so shift 0

  void initDemo () {
    for (uint8_t i=0; i<STOPCOUNT; i++) {
      stopState[i] = 0;
      stopShift[i] = 0;
    }
    midi::send(STOP_CHANNEL, calcStop(0, stopShift[1]), KEY_ON);
    midi::send(STOP_CHANNEL, calcStop(1, stopShift[1]), KEY_ON);
    midi::send(STOP_CHANNEL, calcStop(2, stopShift[1]), KEY_ON);
  }

  /**
   * Sets up the stop lines and their associated LEDs.
   */
  void init () {
    for (uint8_t i=0; i<STOPCOUNT; i++) {
      pinMode(stopLines[i], INPUT_PULLUP);
      pinMode(stopLeds[i], OUTPUT);
    }
  }

  /**
   * Scans the stop lines and updates their state.
   */
  void scan () {
    for (uint8_t i=0; i<STOPCOUNT; i++) {
      int8_t debounce = stopDebounce[i];
      int8_t old = stopState[i];
      if (debounce > 1) {
        stopDebounce[i]--;
        return;
      }
      if (debounce < 0) {
        stopDebounce[i]++;
        return;
      }
      uint8_t current = digitalRead(stopLines[i]);

      if (current == 0 && debounce == 0) {
        Serial.println("Ja");
        stopDebounce[i] = DEBOUNCE;
        if (old == 0) {
          stopState[i] = 1;
          digitalWrite(stopLeds[i], HIGH);
          midi::send(STOP_CHANNEL, calcStop(i, stopShift[i]), KEY_ON);
        } else {
          stopState[i] = 0;
          digitalWrite(stopLeds[i], LOW);
          midi::send(STOP_CHANNEL, calcStop(i, stopShift[i]), KEY_ON);          
        }
        #ifdef DEBUG
          Serial.println("Stop " + String(i) + " changed to " + String(stopState[i]) + " with shift " + String(stopShift[i]));
        #endif
      }
      if (current == 1 && debounce == 1) {
        stopDebounce[i] = -DEBOUNCE;
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
}