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

/**
 * A UDP MIFI implementation, insipred by ipMIDI (https://github.com/lathoub/Arduino-ipMIDI)
 * For simplicity this implementation doesn't pack the command and channel into a single byte,
 * but instead sends them as separate bytes. This makes it easier to work with the MIDI messages
 * on the receiving end.
 */

#include <Arduino.h>
#include <generic.h>
#include <wifiMidi.h>
#include <midi.h>

#ifdef ARDUINO_GIGA_M7
#include <WiFi.h>
#endif

#define WIFI_KEY 11
#define WIFI_LED 10

#define MIDI_UDP_PORT 21928

#define SSID "PipeOrgan"
#define PASSWORD "PlayMusic"

namespace wifiMidi {
  typedef struct {
    uint8_t channel;
    uint8_t command;
    uint8_t keyNum;
    uint8_t val;
  } MidiMessage;

  int8_t wifi_on;
  int8_t debounce;

  void enable();
  void disable();

#ifdef ARDUINO_GIGA_M7
  char ssid[] = SSID;  // your network SSID (name)
  char pass[] = PASSWORD; // your network password
  int status = WL_IDLE_STATUS;


/**
   * Sets up the calcant switch and its associated LED.
   */
  void init () {
    pinMode(WIFI_KEY, INPUT_PULLUP);
    pinMode(WIFI_LED, OUTPUT);
    digitalWrite(WIFI_LED, LOW);
  }

  IPAddress remoteIp;

  /**
   * Setup the WiFi MIDI connection. This will block until a connection is established.
   */
  void enable () {
    static const char ssid[] = SSID;  // your network SSID (name)
    static const char pass[] = PASSWORD; // your network password
    if (WiFi.status() == WL_NO_SHIELD) {
      #ifdef DEBUG
        Serial.println("Communication with WiFi module failed!");
      #endif
      // don't continue
      while (true);
    }

    // attempt to connect to WiFi network:
    while (status != WL_CONNECTED) {
      #ifdef DEBUG
        Serial.print("Attempting to connect to SSID: ");
        Serial.println(ssid);
      #endif
      // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
      status = WiFi.begin(ssid, pass);
      // wait 10 seconds for connection:
      delay(10000);
    }
    remoteIp = WiFi.gatewayIP();
      #ifdef DEBUG
        Serial.print("Connected to WiFi network with IP address: ");
        Serial.println(WiFi.localIP());
        Serial.print("Remote IP address: ");
        Serial.println(remoteIp);
      #endif
  }

  void disable () {
    status = WL_IDLE_STATUS;
    WiFi.end();
  }

  /**
   * Send a MIDI message over UDP to the remote IP address.
   * @param channel The MIDI channel (0-15)
   * @param command The MIDI command (e.g. 0x90 for note on)
   * @param keyNum The MIDI key number (0-127)
   * @return MIDI_OK if the message was sent successfully, MIDI_NOTCONN if not connected, or MIDI_ERROR if there was an error sending the message
   */
  int send (uint8_t channel, uint8_t command, uint8_t keyNum) {
    if (status != WL_CONNECTED) {
      return MIDI_NOTCONN;
    }
    MidiMessage message;
    message.channel = channel;
    message.command = command;
    message.keyNum = keyNum;
    message.val = 0x7F;

    WiFiUDP udp;
    if (!udp.beginPacket(remoteIp, MIDI_UDP_PORT)) {
      return MIDI_NOTCONN;
    }
    udp.write((uint8_t*)&message, sizeof(MidiMessage));
    if (!udp.endPacket()) {
      return MIDI_ERROR;
    }
    return MIDI_OK;
  }

  /**
   * If the blower is off we blink the led to indicate that the organ is not making sound,
   * otherwise we keep it on to indicate that the organ is making sound.
   */
  void blinkLed () {
    if (wifi_on < 1) {
      if (millis() % 1000 < 500) {
        digitalWrite(WIFI_LED, HIGH);
      } else {
        digitalWrite(WIFI_LED, LOW);
      }
    } else {
      digitalWrite(WIFI_LED, HIGH);
    }
  }

  /**
   * Scans the calcant switch and updates its state.
   */
  void scan () {
    // debounce timers
    if (debounce > 1) {
      debounce--;
      return;
    }
    if (debounce < 0) {
      debounce++;
      return;
    }
    // on key-down (after debounce) toggle the state
    uint8_t current = digitalRead(WIFI_KEY);
    if (current == 0 && debounce == 0) {
      if (wifi_on == 0) {
        debounce = DEBOUNCE;
        wifi_on = 1;
        enable();
      } else {
        debounce = -DEBOUNCE;
        wifi_on = 0;
        disable();
      }
    }
    // key-up is just debounced, no action
    if (current == 1 && debounce == 1) {
      debounce = -DEBOUNCE;
    }
  }

#else
  void enable () { /* noOp */ };
  void disable () { /* noOp */ };
  int send(uint8_t channel, uint8_t command, uint8_t keyNum) { return MIDI_UNAVAIL; };
  void scan () { /* noOp */ };
  void blinkLed () { /* noOp */ };
#endif

}