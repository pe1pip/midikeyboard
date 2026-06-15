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
#define STOPSHIFTS 3

// -1
#define STOP0 23 // SW1
#define STOP1 27 // SW2
#define STOP2 31 // SW3

// 0
#define STOP3 2
#define STOP4 4
#define STOP5 6

// +1
#define STOP6 22
#define STOP7 26
#define STOP8 30

// -1
#define STOP0LED 25
#define STOP1LED 29
#define STOP2LED 33

// 0
#define STOP3LED 3
#define STOP4LED 5
#define STOP5LED 7

// +1
#define STOP6LED 24
#define STOP7LED 28
#define STOP8LED 32

namespace stops {
  static const uint8_t stopGroup[STOPCOUNT][STOPSHIFTS] = {{STOP0, STOP3, STOP6}, {STOP1, STOP4, STOP7}, {STOP2, STOP5, STOP8}};
  static const uint8_t ledGroup[STOPCOUNT][STOPSHIFTS] = {{STOP0LED, STOP3LED, STOP6LED}, {STOP1LED, STOP4LED, STOP7LED}, {STOP2LED, STOP5LED, STOP8LED}};

  int8_t stopState[STOPCOUNT][STOPSHIFTS] = { 0 }; // we start with all stops on, so the organ makes sound when powered on
  int8_t stopDebounce[STOPCOUNT][STOPSHIFTS] = { 0 };

  void doStopOn(uint8_t stop, uint8_t shift);
  void doStopOff(uint8_t stop, uint8_t shift);

  void initDemo () {
    init();
    for (uint8_t i=0; i<STOPCOUNT; i++) {
      doStopOn(stopGroup[i][0], 1);
    }
  }

  /**
   * Sets up the stop lines and their associated LEDs.
   */
  void init () {
    for (uint8_t i=0; i<STOPCOUNT; i++) {
      for (uint8_t j = 0; j<STOPSHIFTS; j++) {
        pinMode(stopGroup[i][j], INPUT_PULLUP);
        pinMode(ledGroup[i][j], OUTPUT);
      }
    }
  }

  /**
   * Scans the stop lines and updates their state.
   */
  void scan () {
    for (uint8_t stop=0; stop<STOPCOUNT; stop++) {
      for (uint8_t shift = 0; shift < STOPSHIFTS; shift++) {
        int8_t debounce = stopDebounce[stop][shift];
        int8_t old = stopState[stop][shift];
        if (debounce > 1) {
          stopDebounce[stop][shift]--;
          return;
        }
        if (debounce < 0) {
          stopDebounce[stop][shift]++;
          return;
        }
        uint8_t current = digitalRead(stopGroup[stop][shift]);

        if (current == 0 && debounce == 0) {
          stopDebounce[stop][shift] = DEBOUNCE;
          if (old == 0) {
            doStopOn(stop, shift);
          } else {
            doStopOff(stop, shift);        
          }
          #ifdef DEBUG
            Serial.println("Stop " + String(stop) + " changed to " + String(stopState[stop][shift]));
          #endif
        }
        if (current == 1 && debounce == 1) {
          stopDebounce[stop][shift] = -DEBOUNCE;
        }
      }
    }
  }

  /**
   * when a stop is truned on
   * @param stop - the stop number (0..STOPCOUNT)
   * @param shift - the shft (0..STOPSHIFTS)
   */
  void doStopOn (uint8_t stop, uint8_t shift) {
    // first send the midi for on
    stopState[stop][shift] = 1;
    digitalWrite(ledGroup[stop][shift], HIGH);
    midi::send(STOP_CHANNEL, calcStop(stop, shift), KEY_ON);
    // then just for redundancy, send the off for any other shift of this stop
    for (uint8_t i = 0; i < STOPSHIFTS; i++){
      if (i != shift) {
        // check if the stop is currently in another shift, if so, that's now off
        if (stopState[stop][i] != 0) {
          doStopOff(stop, i);
        }
      }
    }
  }

  /**
   * turn a stop off
   * @param stop - the stop numer (0..STOPCOUNT)
   * @param shift - the stop shift (0..STOPSHIFTS)
   */
  void doStopOff (uint8_t stop, uint8_t shift) {
    stopState[stop][shift] = 0;
    digitalWrite(ledGroup[stop][shift], LOW);
    midi::send(STOP_CHANNEL, calcStop(stop, shift), KEY_OFF);
  }

  /**
   * calculates the MIDI key number for a given stop and shift
   * @param stop The stop number (0..STOPCOUNT)
   * @param shift The shift value (0..STOPSHIFTS)
   * @return The MIDI key number
   */
  inline uint8_t calcStop (uint8_t stop, int8_t shift) {
    return (stop * 8) + 3 + shift;
  }
}