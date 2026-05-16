# Midi Keyboard

## de-mux

The software is designed to use a one-of-eight demux like the 74hc238 to scan a keyboard. With 8 lines this could be up to 5 octave (and a bit).

## Stops

The stops are not multiplexed. The software supports 8 stops. The trick is that each of the stops cycles through 6 options:

- off
- on (no shift)
- on (1 up)
- on (1 down)
- on (2 up)
- on (2 down)

On a stop change the software first sends 'key off', unless the stop is currently off, followed by the key on that corresponds with the stop and shift.
To facilitate that, each stop actually uses 5 values: 2, 3, 4, 5 and 6 with an offset in mutiples of 8 for each stop.

### Stop state

The stop state is stored in an array with a value that corresponds to one of the 6 possible options. The state of the stop buttons is also stored. On a LOW to HIGH transition the stop state is updated and the new value is transmitted.

## MIDI channels

Configurable in code, but defined as:

0: stops
1: keys

## Key state

The (previous) key state is stored in an array. If the value has changed the new value is send over the serial line immediately.
