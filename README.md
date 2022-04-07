# Build a DDR pad with Force-Sensitive Resistors!

## Requirements
- Arduino Leonardo style board. I'd suggest a micro.
- FSRs - I used Interlink FSR 408's -> https://www.interlinkelectronics.com/fsr-408

## Using this code
- Add ArduinoJoystickLibrary to your Arduino libraries -> https://github.com/MHeironimus/ArduinoJoystickLibrary
- Upload to your board
- A0->A3 should be used as inputs.

## Serial communication
Set your panel's configuration via serial. Runs at `115200` baud rate.

- `p #` - Prints data relating to the panel number specified.
- `w # # # # #` - Set a panel configuration in the form `panel number`, `sensor number`, `press threshold`, `depress threshold`, `stepping rate` (steps are typically per millisecond)

Changes will be saved to EEPROM on change and reloaded on start.