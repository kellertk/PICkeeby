# PICkeeby

PS/2 Keyboard to Shift Register Interface for PIC16F716

## Overview

The PIC16 is ancient, but this seemed appropriate for a retro/hobby computer
project. This bridges PS/2 keyboards to an ASCII shift register output. I did
this because bit-banging PS/2 on a slow (1-2 MHz) system is annoying. I didn't
feel like this was too anachronistic since many retro computers used
old microcontrollers for I/O tasks - PIC originally stood for "Peripheral
Interface Controller" after all.

## Features

- **Scancode translation** - US keyboard layout with modifier key support (Shift, Ctrl, Alt)
- **16-byte keystroke buffer**
- **Parity validation**
- **Serial shift register output** for a (74XX595 or a W65C22 or similar)

### Pin Configuration

```
            PIC16F716
             ________
DEBUG_LED - |RA2  RA1| -
          - |RA3  RA0| -
          - |RA4  OSC| - 20MHz Crystal
          - |Vpp  OSC| - 20MHz Crystal
          - |Vss  Vdd| - Power (5V)
KBD_CLOCK - |RB0  RB7| - INTB
          - |RB1  RB6| - SR_CLK  (Shift Register Clock)
          - |RB2  RB5| - SR_DATA (Shift Register Data)
          - |RB3  RB4| - KBD_DATA
             --------
```

This device controls the shift register output clock, which is about 100 khz.
The debug LED on RA2 blinks when a key is buffered.

You can easily use a slower crystal or even run this statically. The clock
frequency is used to calculate the shift register output timing though, so
update the _XTAL_FREQ definition in main.c if you change the crystal frequency.

## Building

This project uses CMake with Microchip XC8 compiler.

1. Configure

    ```bash
    make configure
    ```
2. Build

    ```bash
    make
    ```
3. The output HEX file will be in `build/` directory.

## Theory of Operation

### PS/2 Protocol Reception
1. Falling edge on keyboard clock (RB0) triggers interrupt
2. ISR reads 11-bit frame: start bit, 8 data bits (LSB first), parity, stop bit
3. Validates odd parity and proper start/stop bits
4. Looks up the scancode in translation table

### Scancode Translation
The scancode translation was largely taken from Paul Stoffregen's [PS2Keyboard](https://github.com/PaulStoffregen/PS2Keyboard)
library (and therefore is under the same LGPLv2.1 license).

### Buffering & Output
The PIC controls the shift register output clock, which is paused if new data
arrives from the keyboard during a shift out operation. Pulling INTB low during
the shift out operation is ignored until the shift out is complete.

There is a 16-byte circular buffer for keystrokes. If the buffer is full, new
keystrokes are discarded.

## License
The keymap source is licensed under the LGPLv2.1. See the keymap.c file for details.

The rest of the code is licensed under the MIT License.
