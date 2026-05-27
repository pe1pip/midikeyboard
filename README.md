# Midi Keyboard

[[_TOC_]]

## Introduction

Copyright 2026 Remco Post

[License AGPL](./LICENSE.md)

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

## Keyborad

The keyboard is a simple matrix, 8 input lines, diodes to each key contact and then 8 lines. The current flows from 'Keys in group' to 'Group'.

### Groups

| keys | pin | giga pin |
|------|-----|----------|
| 1-8  | 16  | 53       |
| 9-16 | 1   | 52       |
| 17-24| 15  | 51       |
| 25-32| 2   | 50       |
| 33-40| 14  | 49       |
| 41-48| 3   | 48       |
| 49-56| 13  | 47       |
| 57-61| 4   | 46       |

### Keys in group

| key | pin | giga pin |
|-----|-----|----------|
| 1   | 10  | 41       |
| 2   | 5   | 44       |
| 3   | 6   | 42       |
| 4   | 12  | 45       |
| 5   | 11  | 43       |
| 6   | 9   | 39       |
| 7   | 8   | 38       |
| 8   | 7   | 40       |

### Scanning the keyboard

The 'group' lines are pulled up via the internal pullup resister. If a key is depressed and the 'Keys in group' line is 'LOW', then the 'group' line goes 'LOW'. So, normally all 'Key in group' lines are high. Then one by one each is set to 'LOW', and all 'Group' lines are read to see which of the keys are depressed; those will read 'LOW'.