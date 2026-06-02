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

#include <midi.h>

void setupMidi () {
  midi.begin(MIDI_BAUDRATE, SERIAL_8N1);
}

/**
 * Sends a MIDI message on the specified channel with the given key number and value.
 * @param channel The MIDI channel to send the message on (0-15).
 * @param keyNum The MIDI key number to send (0-127).
 * @param midiCommand The MIDI command to send (e.g. KEY_ON or KEY_OFF).
 */
void sendMidi (uint8_t channel, uint8_t keyNum, uint8_t midiCommand) {
  uint8_t command = midiCommand + channel;
  uint8_t velocity = 0x7F;
  uint8_t key = keyNum + KEY_BASE;
  midi.write(command);
  midi.write(velocity);
  midi .write(key);
  #ifdef DEBUG
  Serial.println("Sent MIDI message on channel " + String(channel) + " with command " + String(midiCommand, HEX) + " and key " + String(key, HEX));
  #endif
}
