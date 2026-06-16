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

// MIDI related constants and variables
#define MIDI_IN 8
#define MIDI_OUT 9
#define MIDI_BAUDRATE 31250

#ifdef ARDUINO_GIGA_M7
#define midiUART Serial4 // RX3/TX3
#endif

#ifdef ARDUINO_AVR_MEGA2560
#define midiUART Serial3 // RX3/TX3
#endif

namespace midi {
  void init () {
    midiUART.begin(MIDI_BAUDRATE, SERIAL_8N1);
  }

  /**
   * Sends a MIDI message on the specified channel with the given key number and value.
   * @param channel The MIDI channel to send the message on (0-15).
   * @param keyNum The MIDI key number to send (0-127).
   * @param midiCommand The MIDI command to send (e.g. KEY_ON or KEY_OFF).
   */
  void send (uint8_t channel, uint8_t keyNum, uint8_t midiCommand) {
    uint8_t command = midiCommand + channel;
    uint8_t velocity = 0x7F;
    midiUART.write(command);
    midiUART.write(keyNum);
    midiUART.write(velocity);
    #ifdef DEBUG
    Serial.println("Sent MIDI message on channel " + String(channel) + " with command " + String(midiCommand, HEX) + " and key " + String(keyNum, HEX));
    #endif
  }
}
