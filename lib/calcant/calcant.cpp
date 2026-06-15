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
#include <calcant.h>
#include <midi.h>

#define CALCANT 9
#define CALCANT_LED 8

namespace calcant { 
  int8_t calcant;
  int8_t calcant_debounce;

  /**
   * Sets up the calcant switch and its associated LED.
   */
  void init () {
    pinMode(CALCANT, INPUT_PULLUP);
    pinMode(CALCANT_LED, OUTPUT);
    digitalWrite(CALCANT_LED, LOW);
  }

  /**
   * If the blower is off we blink the led to indicate that the organ is not making sound,
   * otherwise we keep it on to indicate that the organ is making sound.
   */
  void blinkLed () {
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
  void scan () {
    // debounce timers
    if (calcant_debounce > 1) {
      calcant_debounce--;
      return;
    }
    if (calcant_debounce < 0) {
      calcant_debounce++;
      return;
    }
    // on key-down (after debounce) toggle the state
    uint8_t current = digitalRead(CALCANT);
    if (current == 0 && calcant_debounce == 0) {
      if (calcant == 0) {
        calcant_debounce = DEBOUNCE;
        calcant = 1;
        midi::send(STOP_CHANNEL, 0, KEY_ON);
      } else {
        calcant_debounce = -DEBOUNCE;
        calcant = 0;
        midi::send(STOP_CHANNEL, 0, KEY_OFF);
      }
    }
    // key-up is just debounced, no action
    if (current == 1 && calcant_debounce == 1) {
      calcant_debounce = -DEBOUNCE;
    }
  }
}