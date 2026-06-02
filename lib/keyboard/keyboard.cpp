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

#include <keyboard.h>
#include <midi.h>
#include <generic.h>

uint8_t keyLines[8] = {KEY0, KEY1, KEY2, KEY3, KEY4, KEY5, KEY6, KEY7};
uint8_t groupLines[8] = {GROUPLINE0, GROUPLINE1, GROUPLINE2, GROUPLINE3, GROUPLINE4, GROUPLINE5, GROUPLINE6, GROUPLINE7};
int8_t keyState[KEYCOUNT];

/**
 * Sets up the key lines.
 */
void setupKeylines () {
  for (uint8_t i=0; i<sizeof(keyLines); i++) {
    pinMode(keyLines[i], KEY_PINMODE);
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