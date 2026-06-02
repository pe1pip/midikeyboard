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
#include <calcant.h>
#include <midi.h>
#include <generic.h>

int8_t calcant;

/**
 * Sets up the calcant switch and its associated LED.
 */
void setupCalcant () {
  pinMode(CALCANT, INPUT_PULLUP);
  pinMode(CALCANT_LED, OUTPUT);
  digitalWrite(CALCANT_LED, LOW);
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