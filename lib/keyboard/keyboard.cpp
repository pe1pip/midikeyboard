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
#include <keyboard.h>
#include <midi.h>

#define KEY_PINMODE OUTPUT
#define GROUPLINE_PINMODE INPUT_PULLUP

#define KEYCOUNT 64
#define GROUPLINE0 52 // 1
#define GROUPLINE1 53 // 16
#define GROUPLINE2 50 // 2
#define GROUPLINE3 51 // 15
#define GROUPLINE4 48 // 3
#define GROUPLINE5 49 // 14
#define GROUPLINE6 46 // 4
#define GROUPLINE7 47 // 13
#define KEY0 40 // 7
#define KEY1 45 // 12
#define KEY2 43 // 11
#define KEY3 44 // 5
#define KEY4 42 // 6
#define KEY5 38 // 8
#define KEY6 39 // 9
#define KEY7 41 // 10

namespace keyboard {
  uint8_t keyLines[8] = {KEY0, KEY1, KEY2, KEY3, KEY4, KEY5, KEY6, KEY7};
  uint8_t groupLines[8] = {GROUPLINE0, GROUPLINE1, GROUPLINE2, GROUPLINE3, GROUPLINE4, GROUPLINE5, GROUPLINE6, GROUPLINE7};
  int8_t keyState[KEYCOUNT] = { 0 };

  /**
   * Sets up the key lines.
   */
  void init () {
    for (uint8_t i=0; i<sizeof(keyLines); i++) {
      pinMode(keyLines[i], KEY_PINMODE);
      digitalWrite(keyLines[i], HIGH);
    }
    for (uint8_t i=0; i<sizeof(groupLines); i++) {
      pinMode(groupLines[i], GROUPLINE_PINMODE);
    }
  }

  /**
   * Scans the keyboard and updates the state of each key.
   */
  void scan () {
    for (uint8_t line = 0; line < sizeof(keyLines); line++) {
      digitalWrite(keyLines[line], LOW);
      for (uint8_t i=0; i<sizeof(groupLines); i++) {
        uint8_t keyNum = i * 8 + line;
        int8_t old = keyState[keyNum];
        if (old < 0) { // if the key was just released, count up to 0
          keyState[keyNum]++;
          continue;
        }
        if (old > 1) { // if the key was just pressed, count down to 1
          keyState[keyNum]--;
          continue;
        }
        uint8_t val = digitalRead(groupLines[i]); // 0 if the key is pressed, 1 if it's not

        if (old == 0 && val == 0) { // if the key was not pressed and now is
          keyState[keyNum] = DEBOUNCE;
          midi::send(KEY_CHANNEL, keyNum + KEY_BASE, KEY_ON);
          continue;
        }
        if (old == 1 && val == 1) { // if the key was pressed and now is not
          keyState[keyNum] = -DEBOUNCE;
          midi::send(KEY_CHANNEL, keyNum + KEY_BASE, KEY_OFF);
          continue;
        }
      }
      digitalWrite(keyLines[line], HIGH);
    }
  }
}