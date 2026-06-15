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
#include <midi.h>
#include <wifiMidi.h>
#include <keyboard.h>
#include <stops.h>
#include <calcant.h> 

void setup() {
  keyboard::init();
  stops::init();
  calcant::init();
#ifdef DEBUG
  Serial.begin(115200);
  while(!Serial); // wait for serial to be ready
  delay(250); // wait for the serial monitor to open
  Serial.println("Keyboard initialized");
  Serial.println("Debug mode enabled");
  stops::initDemo();
#endif
}

void loop() {
  keyboard::scan();
  stops::scan();
  calcant::scan();
  wifiMidi::scan();
  calcant::blinkLed();
  wifiMidi::blinkLed();
}
