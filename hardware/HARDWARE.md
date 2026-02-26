# PICkeeby Hardware

## Overview

**Architecture:**

```text
                          HOST BUS (6502 or Z80)
                      D0-D7  A0  /CS  SIG1  SIG2
                        |     |   |    |     |
        +---------------+-----+---+----+-----+-----------+
        |               |         |          |           |
        |               |         |     +----+----+      |
        |               |         |     | U5:     |      |
        |               |         |     | 1G14    |      |
        |               |         |     +----+----+      |
        |               |         |          | /SIG2     |
        |    +----------+---------+----------+-----+     |
        |    |       U4: ATF22V10C GAL             |     |
        |    |   (Decoder + Status + OBF/IBF)      |     |
        |    +----+--------+----------+------------+     |
        |         |        |          |                  |
        |     /RD_DATA  LED_STATUS  Signals              |
        |         |        |                             |
+-------+----+    |    +===+===+           +-------------+-------+
| U2: 574    |    |    | (GAL) |           | U3: 373             |
| Output     |<---+    | Status|           | Input               |
| Latch      |         | Buffer|           | Latch               |
+-----+------+         +===+===+           +----------+----------+
      |                    |                          |
      | Q                  |                          | Q
      |                    |                          |
      +--------IDATA-------+----------IDATA----------+
                           |
                        +--+--+
                        | U1  |
                        | PIC |
                        +--+--+
                         /   \
                   PS/2      PS/2
                   Port 1    Port 2
```

---

## Components (4 ICs)

| Ref | Part            | Function                                    |
| --- | --------------- | ------------------------------------------- |
| U1  | PIC16F18344-I/P | Main controller (dual PS/2)                 |
| U2  | 74AHCT574       | Output Latch (PIC → Host)                   |
| U3  | 74AHCT373       | Input Latch (Host → PIC)                    |
| U4  | ATF22V10C-15PU  | Address Decoder / Status Flags / Mode Logic |
| U5  | 74AHCT1G14      | Schmitt-trigger inverter (SIG2 → /SIG2)     |

### Other Components

| Ref    | Part             | Function                     |
| ------ | ---------------- | ---------------------------- |
| C1     | 1µF              | U1 decoupling                |
| C2     | 1µF              | U2 decoupling                |
| C3     | 1µF              | U3 decoupling                |
| C4     | 1µF              | U4 decoupling                |
| C5     | 1µF              | U5 decoupling                |
| R1, R2 | 4.7K             | PS/2 Port 1 pull-ups         |
| R3, R4 | 4.7K             | PS/2 Port 2 pull-ups         |
| R5-R8  | 300Ω             | LED current limiters (×4)    |
| R9     | 10K              | MODE pull-up                 |
| D1     | 3mm LED (yellow) | Chip select indicator        |
| D2     | 3mm LED (red)    | Output buffer full indicator |
| D3     | 3mm LED (red)    | Input buffer full indicator  |
| D4     | 3mm LED (green)  | Host polling indicator       |
| J1     | Mini-DIN-6       | PS/2 Port 1 connector        |
| J2     | Mini-DIN-6       | PS/2 Port 2 connector        |
| J3     | 1x3 header       | Mode select jumper           |
| J4     | 1x15 header      | Host bus connector           |
| J5     | 1x6 header       | ICSP programming header      |

---

## Signal Reference

| Signal     | Description                                                    |
| ---------- | -------------------------------------------------------------- |
| HDATA0-7   | Host data bus                                                  |
| IDATA0-7   | Internal data bus (between PIC and latches)                    |
| A0         | Address bit 0 from host (to GAL only)                          |
| A0_REG     | Registered A0 from GAL (to PIC RA3)                            |
| SIG1       | R/W (6502) or /RD (Z80)                                        |
| SIG2       | Φ2 (6502) or /WR (Z80)                                         |
| /SIG2      | Inverted SIG2 from U5 (GAL registered clock)                   |
| /RD_DATA   | Active LOW: host reading data port                             |
| LED_STATUS | Active HIGH: host reading status                               |
| /IRQ       | Active LOW: interrupt to host                                  |
| CLK_OUT    | Rising edge: latch output data, set OBF                        |
| /OE_IN     | Active LOW: enable input latch outputs                         |
| /IBF_CLR   | Active LOW: clear IBF flag (held until next clock edge)        |
| LE_IN      | Latch Enable for input buffer: HIGH=transparent, LOW=latched   |
| OBF        | Output Buffer Full (status bit 0)                              |
| IBF        | Input Buffer Full (status bit 1, registered)                   |
| /AUX       | Auxiliary output buffer Full (status bit 5) - data from Port 2 |
| PS2_CLK1   | PS/2 Port 1 clock                                              |
| PS2_DATA1  | PS/2 Port 1 data                                               |
| PS2_CLK2   | PS/2 Port 2 clock                                              |
| PS2_DATA2  | PS/2 Port 2 data                                               |

---

## Bus Mode Selection (J3)

```text
J3: Mode Select (active LOW selects mode)
  [1-2] = 6502 mode (MODE pin LOW)
  [2-3] = Z80 mode  (MODE pin HIGH)

       6502    Z80
        ■──■    □       (shown: 6502 mode)
        1  2    3
           └────┘

Pin 1: GND
Pin 2: MODE (to U4)
Pin 3: +5V via 10K pull-up (R9)
```

### SIG1 and SIG2

These set the type of bus for the host: a 6502-style bus or a Z80-style bus.

| J4 Pin | Signal | 6502 Mode        | Z80 Mode        |
| ------ | ------ | ---------------- | --------------- |
| 10     | SIG1   | R/W (HIGH=read)  | /RD (LOW=read)  |
| 12     | SIG2   | Φ2 (clock phase) | /WR (LOW=write) |

---

## Register Map

The A0 line selects between data port and status/command port. This is the same
as the i8042.

| A0  | R/W                | Internal address decode result |
| --- | ------------------ | ------------------------------ |
| 0   | Read Data port     | /RD_DATA                       |
| 0   | Write Data port    | (write with A0=0)              |
| 1   | Read Status port   | LED_STATUS                     |
| 1   | Write Command port | (write with A0=1)              |

---

## U1: PIC16F18344-I/P (DIP-20)

```text
              +-----------+
        +5V --|1        20|-- GND
       /AUX --|2        19|-- CLK_OUT
        IBF --|3        18|-- /OE_IN
         A0 --|4        17|-- /IBF_CLR
     IDATA0 --|5        16|-- IDATA1
     IDATA2 --|6        15|-- IDATA3
     IDATA4 --|7        14|-- IDATA5
     IDATA6 --|8        13|-- PS2_CLK1
     IDATA7 --|9        12|-- PS2_DATA1
  PS2_DATA2 --|10       11|-- PS2_CLK2
              +-----------+
```

### U1 Pin Connections

| Pin | Name | Signal    | To                          |
| --- | ---- | --------- | --------------------------- |
| 1   | VDD  | +5V       | +5V, J5-2                   |
| 2   | RA5  | /AUX      | U4-8                        |
| 3   | RA4  | IBF       | U4-16                       |
| 4   | RA3  | A0_REG    | U4-14                       |
| 5   | RC5  | IDATA0    | U2-2, U3-2                  |
| 6   | RC4  | IDATA2    | U2-4, U3-6                  |
| 7   | RC3  | IDATA4    | U2-6, U3-19                 |
| 8   | RC6  | IDATA6    | U2-8, U3-15                 |
| 9   | RC7  | IDATA7    | U2-9, U3-12                 |
| 10  | RB7  | PS2_DATA2 | J2-1, R4 to +5V             |
| 11  | RB6  | PS2_CLK2  | J2-5, R3 to +5V             |
| 12  | RB5  | PS2_DATA1 | J1-1, R2 to +5V             |
| 13  | RB4  | PS2_CLK1  | J1-5, R1 to +5V             |
| 14  | RC2  | IDATA5    | U2-7, U3-16                 |
| 15  | RC1  | IDATA3    | U2-5, U3-9                  |
| 16  | RC0  | IDATA1    | U2-3, U3-5                  |
| 17  | RA2  | /IBF_CLR  | U4-5                        |
| 18  | RA1  | /OE_IN    | U3-1, J5-5 (ICSPCLK)        |
| 19  | RA0  | CLK_OUT   | U4-9, U2-11, J5-4 (ICSPDAT) |
| 20  | VSS  | GND       | GND, J5-3                   |

**C1 (1µF):** Pin 1 to Pin 20

**A0 Latching:** The GAL captures A0 into a registered output (A0_REG) on the
falling edge of PHI2 (6502) or falling edge of /WR (Z80), using the /SIG2 clock
from U5. The PIC reads A0_REG directly from RA3 to distinguish between data
register writes (A0=0) and command register writes (A0=1). This replaces the
previous CLC1-based approach and provides hundreds of nanoseconds of setup
margin for A0 capture.

## U2: 74AHCT574 - Output Latch (SOIC-20)

PIC writes scan codes here. Rising edge of CLK_OUT latches data. Host reads when
/RD_DATA enables outputs.

```text
           +-----------+
/RD_DATA --|1        20|-- +5V
  IDATA0 --|2        19|-- HDATA0
  IDATA1 --|3        18|-- HDATA1
  IDATA2 --|4        17|-- HDATA2
  IDATA3 --|5        16|-- HDATA3
  IDATA4 --|6        15|-- HDATA4
  IDATA5 --|7        14|-- HDATA5
  IDATA6 --|8        13|-- HDATA6
  IDATA7 --|9        12|-- HDATA7
     GND --|10       11|-- ←CLK_OUT
           +-----------+
```

### U2 Pin Connections

| Pin | Name | Signal   | To                 |
| --- | ---- | -------- | ------------------ |
| 1   | OEB  | /RD_DATA | U4-23              |
| 2   | D0   | IDATA0   | U1-5, U3-2         |
| 3   | D1   | IDATA1   | U1-16, U3-5        |
| 4   | D2   | IDATA2   | U1-6, U3-6         |
| 5   | D3   | IDATA3   | U1-15, U3-9        |
| 6   | D4   | IDATA4   | U1-7, U3-19        |
| 7   | D5   | IDATA5   | U1-14, U3-16       |
| 8   | D6   | IDATA6   | U1-8, U3-15        |
| 9   | D7   | IDATA7   | U1-9, U3-12        |
| 10  | GND  | GND      | GND                |
| 11  | CLK  | CLK_OUT  | U1-19, U4-9        |
| 12  | Q7   | HDATA7   | J4-8, U3-13        |
| 13  | Q6   | HDATA6   | J4-7, U3-14        |
| 14  | Q5   | HDATA5   | J4-6, U3-17, U4-21 |
| 15  | Q4   | HDATA4   | J4-5, U3-18        |
| 16  | Q3   | HDATA3   | J4-4, U3-8         |
| 17  | Q2   | HDATA2   | J4-3, U3-7         |
| 18  | Q1   | HDATA1   | J4-2, U3-4, U4-18  |
| 19  | Q0   | HDATA0   | J4-1, U3-3, U4-19  |
| 20  | VCC  | +5V      | +5V                |

**C2 (1µF):** Pin 20 to Pin 10

---

## U3: 74AHCT373 - Input Latch (SOIC-20)

Host writes commands here. Transparent when LE=HIGH, latches when LE goes LOW.

```text
         +-----------+
/OE_IN --|1        20|-- +5V
IDATA0 --|2        19|-- IDATA4
HDATA0 --|3        18|-- HDATA4
HDATA1 --|4        17|-- HDATA5
IDATA1 --|5        16|-- IDATA5
IDATA2 --|6        15|-- IDATA6
HDATA2 --|7        14|-- HDATA6
HDATA3 --|8        13|-- HDATA7
IDATA3 --|9        12|-- IDATA7
   GND --|10       11|-- LE_IN
         +-----------+
```

### U3 Pin Connections

| Pin | Name | Signal | To                 |
| --- | ---- | ------ | ------------------ |
| 1   | OEB  | /OE_IN | U1-18              |
| 2   | Q0   | IDATA0 | U1-5, U2-2         |
| 3   | D0   | HDATA0 | J4-1, U2-19, U4-19 |
| 4   | D1   | HDATA1 | J4-2, U2-18, U4-18 |
| 5   | Q1   | IDATA1 | U1-16, U2-3        |
| 6   | Q2   | IDATA2 | U1-6, U2-4         |
| 7   | D2   | HDATA2 | J4-3, U2-17        |
| 8   | D3   | HDATA3 | J4-4, U2-16        |
| 9   | Q3   | IDATA3 | U1-15, U2-5        |
| 10  | GND  | GND    | GND                |
| 11  | LE   | LE_IN  | U4-20              |
| 12  | Q4   | IDATA7 | U1-9, U2-9         |
| 13  | D4   | HDATA7 | J4-8, U2-12        |
| 14  | D5   | HDATA6 | J4-7, U2-13        |
| 15  | Q5   | IDATA6 | U1-8, U2-8         |
| 16  | Q6   | IDATA5 | U1-14, U2-7        |
| 17  | D6   | HDATA5 | J4-6, U2-14, U4-21 |
| 18  | D7   | HDATA4 | J4-5, U2-15        |
| 19  | Q7   | IDATA4 | U1-7, U2-6         |
| 20  | VCC  | +5V    | +5V                |

**C3 (1µF):** Pin 20 to Pin 10

---

## U4: ATF22V10C - GAL (DIP-24)

Combines address decoder, status buffer, mode-switching logic, and buffer flags.
Pin 1 is the registered clock input, driven by /SIG2 from U5. IBF and A0_REG are
registered outputs clocked on /SIG2 rising edge (= SIG2 falling edge). OBF is a
combinational SR-latch. The GAL's asynchronous reset (AR) is wired to /IBF_CLR,
allowing the PIC to clear IBF (and A0_REG) immediately without waiting for a
clock edge.

```text
           +-----------+
   /SIG2 --|1        24|-- +5V
    MODE --|2        23|-- /RD_DATA
      A0 --|3        22|-- LED_STATUS
    SIG1 --|4        21|-- HDATA5
/IBF_CLR --|5        20|-- LE_IN
    SIG2 --|6        19|-- HDATA0
     /CS --|7        18|-- HDATA1
    /AUX --|8        17|-- OBF
 CLK_OUT --|9        16|-- IBF
         x-|10       15|-- /IRQ
         x-|11       14|-- A0_REG
     GND --|12       13|-- GND (/OE)
           +-----------+
```

### U4 Pin Connections

| Pin | Name | Signal     | To                   |
| --- | ---- | ---------- | -------------------- |
| 1   | CLK  | /SIG2      | U5 output            |
| 2   | I    | MODE       | J3-2                 |
| 3   | I    | A0         | J4-9                 |
| 4   | I    | SIG1       | J4-10                |
| 5   | I    | /IBF_CLR   | U1-17                |
| 6   | I    | SIG2       | J4-12, U5 input      |
| 7   | I    | /CS        | J4-11, R5 to D1      |
| 8   | I    | /AUX       | U1-2                 |
| 9   | I    | CLK_OUT    | U1-19, U2-11         |
| 10  | I    | nc         | GND                  |
| 11  | I    | nc         | GND                  |
| 12  | GND  | GND        | GND                  |
| 13  | I/O  | /OE        | GND (always enabled) |
| 14  | I/O  | A0_REG     | U1-4                 |
| 15  | I/O  | /IRQ       | J4-13                |
| 16  | I/O  | IBF        | U1-3, R7 to D3 anode |
| 17  | I/O  | OBF        | R6 to D2 anode       |
| 18  | I/O  | HDATA1     | J4-2, U2-18, U3-4    |
| 19  | I/O  | HDATA0     | J4-1, U2-19, U3-3    |
| 20  | I/O  | LE_IN      | U3-11                |
| 21  | I/O  | HDATA5     | J4-6, U2-14, U3-14   |
| 22  | I/O  | LED_STATUS | R8 to D4 anode       |
| 23  | I/O  | /RD_DATA   | U2-1                 |
| 24  | VCC  | +5V        | +5V                  |

**C4 (1µF):** Pin 24 to Pin 12

---

## U5: 74AHCT1G14 - Schmitt-Trigger Inverter (SOT-23-5)

Inverts SIG2 to produce /SIG2 for the GAL registered clock input. The Schmitt
trigger provides clean edges from the host bus clock/write signal.

| Pin | Name | Signal | To    |
| --- | ---- | ------ | ----- |
| 1   | A    | SIG2   | J4-12 |
| 2   | GND  | GND    | GND   |
| 3   | Y    | /SIG2  | U4-1  |
| 5   | VCC  | +5V    | +5V   |

**C5 (1µF):** Pin 5 to Pin 2

---

## J1: PS/2 Port 1 Connector (Mini-DIN-6)

Female, front view:

```text
       5   6
      3     4
        1 2
```

| Pin | Signal    | To               |
| --- | --------- | ---------------- |
| 1   | PS2_DATA1 | U1-12, R2 to +5V |
| 3   | GND       | GND              |
| 4   | +5V       | +5V              |
| 5   | PS2_CLK1  | U1-13, R1 to +5V |

---

## J2: PS/2 Port 2 Connector (Mini-DIN-6)

Female, front view:

```text
       5   6
      3     4
        1 2
```

| Pin | Signal    | To               |
| --- | --------- | ---------------- |
| 1   | PS2_DATA2 | U1-10, R4 to +5V |
| 3   | GND       | GND              |
| 4   | +5V       | +5V              |
| 5   | PS2_CLK2  | U1-11, R3 to +5V |

---

## J3: Mode Select (1x3 Header)

| Pin | Signal | To              |
| --- | ------ | --------------- |
| 1   | GND    | GND             |
| 2   | MODE   | U4-2            |
| 3   | +5V    | R9 (10K) to +5V |

**Jumper Position:**

- Pins 1-2: 6502 mode (MODE = LOW)
- Pins 2-3: Z80 mode (MODE = HIGH via R9)

---

## J4: Host Connector (1x15 Header)

```text
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |1|2|3|4|5|6|7|8|9|A|B|C|D|E|F|
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   D D D D D D D D A S / S / + G
   0 1 2 3 4 5 6 7 0 1 C 2 I 5 N
                       S   R V D
                           Q
```

| Pin | Signal | 6502 | Z80  | To                          |
| --- | ------ | ---- | ---- | --------------------------- |
| 1   | D0     | D0   | D0   | HDATA0: U2-19, U3-3, U4-19  |
| 2   | D1     | D1   | D1   | HDATA1: U2-18, U3-4, U4-18  |
| 3   | D2     | D2   | D2   | HDATA2: U2-17, U3-7         |
| 4   | D3     | D3   | D3   | HDATA3: U2-16, U3-8         |
| 5   | D4     | D4   | D4   | HDATA4: U2-15, U3-18        |
| 6   | D5     | D5   | D5   | HDATA5: U2-14, U3-17, U4-21 |
| 7   | D6     | D6   | D6   | HDATA6: U2-13, U3-14        |
| 8   | D7     | D7   | D7   | HDATA7: U2-12, U3-13        |
| 9   | A0     | A0   | A0   | U4-3                        |
| 10  | SIG1   | R/W  | /RD  | U4-4                        |
| 11  | /CS    | /CS  | /CS  | U4-7, R5 to D1              |
| 12  | SIG2   | Φ2   | /WR  | U4-6, U5-1                  |
| 13  | /IRQ   | /IRQ | /INT | U4-15                       |
| 14  | +5V    | +5V  | +5V  | +5V                         |
| 15  | GND    | GND  | GND  | GND                         |

---

## J5: ICSP Header (1x6)

Standard Microchip ICSP pinout:

```text
  +-+-+-+-+-+-+
  |1|2|3|4|5|6|
  +-+-+-+-+-+-+
   M V G P P n
   C D S G G c
   L D S D C
   R
```

| Pin | Signal   | To                   |
| --- | -------- | -------------------- |
| 1   | MCLR/VPP | nc (active LVP mode) |
| 2   | VDD      | +5V                  |
| 3   | VSS      | GND                  |
| 4   | ICSPDAT  | U1-19 (RA0/CLK_OUT)  |
| 5   | ICSPCLK  | U1-18 (RA1//OE_IN)   |
| 6   | nc       | -                    |

**Note:** The PIC uses Low-Voltage Programming (LVP) mode with MCLR disabled.
RA3 is repurposed as the A0_REG input from the GAL. Pin 1 of the ICSP header is
not connected; programming uses only ICSPDAT/ICSPCLK. Do not use high-voltage
programming mode.

---

## LEDs: Status Indicators

| LED | Pin   | Signal     | Color  | Indicates                       |
| --- | ----- | ---------- | ------ | ------------------------------- |
| D1  | J4-11 | /CS        | Yellow | Chip selected (via R5)          |
| D2  | U4-17 | OBF        | Red    | PIC has data waiting for host   |
| D3  | U4-16 | IBF        | Red    | Host has sent command to PIC    |
| D4  | U4-22 | LED_STATUS | Green  | Host is polling status register |

---

## BOM (5 ICs + Support)

### ICs

| Ref | Part            | Package  | Function                      |
| --- | --------------- | -------- | ----------------------------- |
| U1  | PIC16F18344-I/P | DIP-20   | Controller (dual PS/2)        |
| U2  | 74AHCT574       | SOIC-20  | Output Latch                  |
| U3  | 74AHCT373       | SOIC-20  | Input Latch                   |
| U4  | ATF22V10C-xxPU  | DIP-24   | GAL (Decoder/Status/Flags)    |
| U5  | 74AHCT1G14      | SOT-23-5 | Schmitt inverter (SIG2→/SIG2) |

### Passive Components

| Ref    | Value | Package | Function               |
| ------ | ----- | ------- | ---------------------- |
| C1-C5  | 1µF   | Ceramic | Decoupling (×5)        |
| R1, R2 | 4.7K  | 1/4W    | PS/2 Port 1 pull-ups   |
| R3, R4 | 4.7K  | 1/4W    | PS/2 Port 2 pull-ups   |
| R5-R8  | 300Ω  | 1/4W    | LED current limit (×4) |
| R9     | 10K   | 1/4W    | MODE pull-up           |

### Connectors & Indicators

| Ref | Part                | Function    |
| --- | ------------------- | ----------- |
| J1  | Mini-DIN-6 female   | PS/2 Port 1 |
| J2  | Mini-DIN-6 female   | PS/2 Port 2 |
| J3  | 1×3 header + jumper | Mode select |
| J4  | 1×15 header         | Host bus    |
| J5  | 1×6 header          | ICSP        |
| D1  | 3mm LED (yellow)    | Chip select |
| D2  | 3mm LED (red)       | OBF         |
| D3  | 3mm LED (red)       | IBF         |
| D4  | 3mm LED (green)     | Status read |

---

## Data Flow

### PS/2 activity (Port 1)

1. PIC sets /AUX = LOW (data from Port 1)
2. PIC disables U3 outputs (/OE_IN=HIGH)
3. PIC drives IDATA0-7 with scan code
4. PIC pulses CLK_OUT HIGH
5. Rising edge latches into U2, sets OBF (in GAL), asserts /IRQ
6. Host reads data port → /RD_DATA clears OBF (async reset in GAL)

### PS/2 activity (Port 2)

1. PIC sets /AUX = HIGH (data from Port 2)
2. PIC disables U3 outputs (/OE_IN=HIGH)
3. PIC drives IDATA0-7 with scan code
4. PIC pulses CLK_OUT HIGH
5. Rising edge latches into U2, sets OBF (in GAL), asserts /IRQ
6. Host reads status → sees /AUX=1 (bit 5), knows data is from Port 2
7. Host reads data port → /RD_DATA clears OBF (async reset in GAL)

### Host sends command (6502 mode)

1. Host drives D0-D7 with command, sets A0, R/W=LOW
2. During Φ2 HIGH: LE_IN=HIGH (373 transparent, tracking data bus)
3. **Φ2 falls:** LE_IN→LOW (373 latches valid data). /SIG2 rises ~8ns later.
4. GAL clocks registered outputs: IBF captures write condition, A0_REG captures
   A0
5. PIC sees IBF=HIGH, reads A0_REG from RA3, enables U3 (/OE_IN=LOW), reads
   IDATA
6. PIC pulses /IBF_CLR LOW, triggering GAL async reset (clears IBF immediately)

### Host sends command (Z80 mode)

1. Host drives D0-D7 with command, sets A0, /CS=LOW
2. **/WR falls:** /SIG2 rises ~8ns later. GAL clocks: IBF=1, A0_REG captures A0.
   LE_IN=HIGH (373 transparent, tracking data bus)
3. Z80 drives data during /WR LOW. 373 tracks.
4. **/WR rises:** LE_IN→LOW (373 latches data)
5. PIC sees IBF=HIGH, reads A0_REG from RA3, enables U3 (/OE_IN=LOW), reads
   IDATA
6. PIC pulses /IBF_CLR LOW, triggering GAL async reset (clears IBF immediately)

### Host reads status (both modes)

1. Host performs read with A0=1
2. LED_STATUS enables GAL tri-state outputs on HDATA0, HDATA1, HDATA5
3. GAL drives OBF→D0, IBF→D1, /AUX→D5
