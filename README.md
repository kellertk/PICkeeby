# PICkeeby

PS/2 Keyboard/Mouse Controller for Homebrew Computers

## Overview

PICkeeby emulates an i8042 keyboard controller, bridging PS/2 devices to a host
computer's I/O bus. It supports both 6502 and Z80 bus timing via a mode jumper,
and handles two PS/2 ports (keyboard and mouse) simultaneously.

The name is a nod to PIC's origin as "Peripheral Interface Controller" -- using
a small microcontroller for I/O duties is a time-honored tradition in computer
design.

## Hardware

- **MCU:** PIC16F18344 at 32 MHz (internal oscillator)
- **GAL:** ATF22V10C for address decoding, status flags (OBF/IBF), A0
  registration, and IRQ generation
- **Latches:** 74AHCT574 (output) and 74AHCT373 (input) for bus interfacing
- **Inverter:** 74AHCT1G14 derives the GAL register clock from the bus clock

Accent on the "homebrew" -- this is a through-hole design on a two-layer PCB.
Accent on the "HC" -- the bus interface is 5V TTL compatible.

### Bus Compatibility

The MODE jumper selects between:

- **6502 mode:** active-high clock (PHI2), active-high R/W
- **Z80 mode:** active-low /WR, active-low /RD

No minimum bus speed -- the design supports fully static operation. Maximum bus
speed is approximately 33 MHz (limited by GAL propagation delay).

## Building

Requires XC8 (Microchip C compiler) and galette (PLD assembler). Run
`./configure` (or `.\configure.ps1` on Windows) first to detect tools and
generate `config.mk`.

```shell
make              # Build firmware + PLD
make firmware     # Build PIC firmware only
make pld          # Build GAL/PLD only
make lint         # Lint markdown files
make flash        # Flash firmware via MPLAB IPE
make flash-pld    # Flash GAL via minipro
```

## Theory of Operation

The host computer sees two I/O ports at consecutive addresses:

- **Port 0x60 (A0=0):** Data register (read/write)
- **Port 0x64 (A0=1):** Status register (read) / Command register (write)

This matches the standard i8042 keyboard controller interface.

### Data Flow

**PS/2 device to host:** PS/2 clock edges trigger an interrupt-on-change ISR
that shifts in scan code bytes. The i8042 emulation layer queues them for the
host. When the output latch is free, a byte is clocked into the 74HCT574 and OBF
(Output Buffer Full) is set in the GAL, asserting /IRQ.

**Host to PS/2 device:** The host writes to the data or command register. The
GAL captures IBF (Input Buffer Full) and A0 on the bus clock edge. The PIC polls
IBF, reads the byte from the 74HCT373 input latch, and routes it through the
i8042 command processor.

### i8042 Emulation

Supports the standard command set: read/write command byte, port enable/disable,
self-test, port test, write to auxiliary device, and read/write of 32 bytes of
EEPROM-backed internal RAM. Scan code translation and A20 gate control are not
implemented.

## Documentation

- [Firmware Design](src/FIRMWARE.md) -- module details, state machines, pin
  assignments, timing
- [Hardware Design](hardware/HARDWARE.md) -- schematic walkthrough, GAL
  equations, connector pinouts

## License

This work is licensed under [CC BY-NC-SA 4.0](LICENSE). This license does not
permit commercial use.
