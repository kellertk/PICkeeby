# PICkeeby Hardware

## Overview

**Architecture:**
```
                          HOST BUS (6502 or Z80)
                      D0-D7  A0  CSB  SIG1  SIG2
                        |     |   |    |     |
        +---------------+-----+---+----+-----+-----------+
        |               |         |          |           |
        |    +----------+---------+----------+-----+     |
        |    |       U4: ATF22V10C GAL             |     |
        |    |   (Decoder + Status + OBF/IBF)      |     |
        |    +----+--------+----------+------------+     |
        |         |        |          |                  |
        |     RD_DATAB  LED_STATUS  Signals              |
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

| Ref | Part | Function |
|-----|------|----------|
| U1 | PIC16F18344-I/P | Main controller (dual PS/2) |
| U2 | 74HCT574 | Output Latch (PIC → Host) |
| U3 | 74HCT373 | Input Latch (Host → PIC) |
| U4 | ATF22V10C-15PU | Address Decoder / Status Flags / Mode Logic |

**Important:** ATF22V10**C** variant is required!

### Other Components

| Ref | Part | Function |
|-----|------|----------|
| C1, C2 | 100nF, 10nF | U1 decoupling |
| C3, C4 | 100nF, 10nF | U2 decoupling |
| C5, C6 | 100nF, 10nF | U3 decoupling |
| C7, C8 | 100nF, 10nF | U4 decoupling |
| R1 | 1K | A0 series resistor (ICSP protection) |
| R2, R3 | 4.7K | PS/2 Port 1 pull-ups |
| R4, R5 | 4.7K | PS/2 Port 2 pull-ups |
| R6-R9 | 220Ω | LED current limiters (×4) |
| R11 | 10K | MODE pull-up |
| D1 | 3mm LED (green) | Chip select indicator |
| D2 | 3mm LED (red) | Output buffer full indicator |
| D3 | 3mm LED (red) | Input buffer full indicator |
| D4 | 3mm LED (green) | Host polling indicator |
| J1 | Mini-DIN-6 | PS/2 Port 1 connector |
| J2 | Mini-DIN-6 | PS/2 Port 2 connector |
| J3 | 1x3 header | Mode select jumper |
| J4 | 1x15 header | Host bus connector |
| J5 | 1x6 header | ICSP programming header |

---

## Signal Reference

| Signal | Description |
|--------|-------------|
| HDATA0-7 | Host data bus |
| IDATA0-7 | Internal data bus (between PIC and latches) |
| A0 | Address bit 0 from host (directly from J4, and via R1 to PIC) |
| SIG1 | R/WB (6502) or RDB (Z80) |
| SIG2 | Φ2 (6502) or WRB (Z80) |
| RD_DATAB | Active LOW: host reading data port |
| LED_STATUS | Active HIGH: host reading status |
| IRQB | Active LOW: interrupt to host |
| CLK_OUT | Rising edge: latch output data, set OBF |
| OE_INB | Active LOW: enable input latch outputs |
| IBF_CLRB | Active LOW pulse: clear IBF flag |
| LE_IN | Latch Enable for input buffer: HIGH=transparent, LOW=latched |
| OBF | Output Buffer Full (status bit 0) |
| IBF | Input Buffer Full (status bit 1) |
| AUXB | Auxiliary output buffer Full (status bit 5) - data from Port 2 |
| PS2_CLK1 | PS/2 Port 1 clock |
| PS2_DATA1 | PS/2 Port 1 data |
| PS2_CLK2 | PS/2 Port 2 clock |
| PS2_DATA2 | PS/2 Port 2 data |

---

## Bus Mode Selection (J3)

```
J3: Mode Select (active LOW selects mode)
  [1-2] = 6502 mode (MODE pin LOW)
  [2-3] = Z80 mode  (MODE pin HIGH)

       6502    Z80
        ■──■    □       (shown: 6502 mode)
        1  2    3
           └────┘

Pin 1: GND
Pin 2: MODE (to U4)
Pin 3: +5V via 10K pull-up (R11)
```

### SIG1 and SIG2

These set the type of bus for the host: a 6502-style bus or a Z80-style bus.

| J4 Pin | Signal | 6502 Mode | Z80 Mode |
|--------|--------|-----------|----------|
| 10 | SIG1 | R/WB (HIGH=read) | RDB (LOW=read) |
| 12 | SIG2 | Φ2 (clock phase) | WRB (LOW=write) |

---

## Register Map

The A0 line selects between data port and status/command port. This is the same
as the i8042.

| A0 | R/W | Internal address decode result |
|----|-----------|---------------|
| 0 | Read Data port | RD_DATAB |
| 0 | Write Data port | (write with A0=0) |
| 1 | Read Status port | LED_STATUS |
| 1 | Write Command port| (write with A0=1) |

---

## U1: PIC16F18344-I/P (DIP-20)

```
              +-----------+
        +5V --|1        20|-- GND
       AUXB --|2        19|-- CLK_OUT
        IBF --|3        18|-- OE_INB
         A0 --|4        17|-- IBF_CLRB
     IDATA5 --|5        16|-- IDATA0
     IDATA4 --|6        15|-- IDATA1
     IDATA3 --|7        14|-- IDATA2
     IDATA6 --|8        13|-- PS2_CLK1
     IDATA7 --|9        12|-- PS2_DATA1
  PS2_DATA2 --|10       11|-- PS2_CLK2
              +-----------+
```

### Pin Connections

| Pin | Name | Signal | To |
|-----|------|--------|----|
| 1 | VDD | +5V | +5V, J5-2 |
| 2 | RA5 | AUXB | U4-8 |
| 3 | RA4 | IBF | U4-16 |
| 4 | RA3 | A0 | J4-9 via R1 |
| 5 | RC5 | IDATA5 | U2-7, U3-15 |
| 6 | RC4 | IDATA4 | U2-6, U3-12 |
| 7 | RC3 | IDATA3 | U2-5, U3-9 |
| 8 | RC6 | IDATA6 | U2-8, U3-16 |
| 9 | RC7 | IDATA7 | U2-9, U3-19 |
| 10 | RB7 | PS2_DATA2 | J2-1, R5 to +5V |
| 11 | RB6 | PS2_CLK2 | J2-5, R4 to +5V |
| 12 | RB5 | PS2_DATA1 | J1-1, R3 to +5V |
| 13 | RB4 | PS2_CLK1 | J1-5, R2 to +5V |
| 14 | RC2 | IDATA2 | U2-4, U3-6 |
| 15 | RC1 | IDATA1 | U2-3, U3-5 |
| 16 | RC0 | IDATA0 | U2-2, U3-2 |
| 17 | RA2 | IBF_CLRB | U4-5 |
| 18 | RA1 | OE_INB | U3-1, J5-5 (ICSPCLK) |
| 19 | RA0 | CLK_OUT | U2-11, U4-1, J5-4 (ICSPDAT) |
| 20 | VSS | GND | GND, J5-3 |

**C1 (100nF), C2 (10nF):** Pin 1 to Pin 20

**A0 Latching:** The PIC's CLC1 module is configured as a D flip-flop to latch the A0 signal on the rising edge of IBF. This allows the firmware to distinguish between data register writes (A0=0) and command register writes (A0=1). R1 provides current limiting in case a programmer incorrectly applies high voltage to the ICSP header.

## U2: 74HCT574 - Output Latch (DIP-20)

PIC writes scan codes here. Rising edge of CLK_OUT latches data.
Host reads when RD_DATAB enables outputs.

```
           +-----------+
RD_DATAB --|1        20|-- +5V
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

### Pin Connections

| Pin | Name | Signal | To |
|-----|------|--------|----|
| 1 | OEB | RD_DATAB | U4-23 |
| 2 | D0 | IDATA0 | U1-16, U3-2 |
| 3 | D1 | IDATA1 | U1-15, U3-5 |
| 4 | D2 | IDATA2 | U1-14, U3-6 |
| 5 | D3 | IDATA3 | U1-7, U3-9 |
| 6 | D4 | IDATA4 | U1-6, U3-12 |
| 7 | D5 | IDATA5 | U1-5, U3-15 |
| 8 | D6 | IDATA6 | U1-8, U3-16 |
| 9 | D7 | IDATA7 | U1-9, U3-19 |
| 10 | GND | GND | GND |
| 11 | CLK | CLK_OUT | U1-19, U4-1 |
| 12 | Q7 | HDATA7 | J4-8, U3-18 |
| 13 | Q6 | HDATA6 | J4-7, U3-17 |
| 14 | Q5 | HDATA5 | J4-6, U3-14, U4-21 |
| 15 | Q4 | HDATA4 | J4-5, U3-13 |
| 16 | Q3 | HDATA3 | J4-4, U3-8 |
| 17 | Q2 | HDATA2 | J4-3, U3-7 |
| 18 | Q1 | HDATA1 | J4-2, U3-4, U4-18 |
| 19 | Q0 | HDATA0 | J4-1, U3-3, U4-19 |
| 20 | VCC | +5V | +5V |

**C3 (100nF), C4 (10nF):** Pin 20 to Pin 10

---

## U3: 74HCT373 - Input Latch (DIP-20)

Host writes commands here. Transparent when LE=HIGH, latches when LE goes LOW.

```
         +-----------+
OE_INB --|1        20|-- +5V
IDATA0 --|2        19|-- IDATA7
HDATA0 --|3        18|-- HDATA7
HDATA1 --|4        17|-- HDATA6
IDATA1 --|5        16|-- IDATA6
IDATA2 --|6        15|-- IDATA5
HDATA2 --|7        14|-- HDATA5
HDATA3 --|8        13|-- HDATA4
IDATA3 --|9        12|-- IDATA4
   GND --|10       11|-- LE_IN
         +-----------+
```

### Pin Connections

| Pin | Name | Signal | To |
|-----|------|--------|----|
| 1 | OEB | OE_INB | U1-18 |
| 2 | Q0 | IDATA0 | U1-16, U2-2 |
| 3 | D0 | HDATA0 | J4-1, U2-19, U4-19 |
| 4 | D1 | HDATA1 | J4-2, U2-18, U4-18 |
| 5 | Q1 | IDATA1 | U1-15, U2-3 |
| 6 | Q2 | IDATA2 | U1-14, U2-4 |
| 7 | D2 | HDATA2 | J4-3, U2-17 |
| 8 | D3 | HDATA3 | J4-4, U2-16 |
| 9 | Q3 | IDATA3 | U1-7, U2-5 |
| 10 | GND | GND | GND |
| 11 | LE | LE_IN | U4-20 |
| 12 | Q4 | IDATA4 | U1-6, U2-6 |
| 13 | D4 | HDATA4 | J4-5, U2-15 |
| 14 | D5 | HDATA5 | J4-6, U2-14, U4-21 |
| 15 | Q5 | IDATA5 | U1-5, U2-7 |
| 16 | Q6 | IDATA6 | U1-8, U2-8 |
| 17 | D6 | HDATA6 | J4-7, U2-13 |
| 18 | D7 | HDATA7 | J4-8, U2-12 |
| 19 | Q7 | IDATA7 | U1-9, U2-9 |
| 20 | VCC | +5V | +5V |

**C5 (100nF), C6 (10nF):** Pin 20 to Pin 10

---

## U4: ATF22V10C - GAL (DIP-24)

Combines address decoder, status buffer, mode-switching logic, and buffer flags.

```
           +-----------+
 CLK_OUT --|1        24|-- +5V
    MODE --|2        23|-- RD_DATAB
      A0 --|3        22|-- LED_STATUS
    SIG1 --|4        21|-- HDATA5
IBF_CLRB --|5        20|-- LE_IN
    SIG2 --|6        19|-- HDATA0
     CSB --|7        18|-- HDATA1
    AUXB --|8        17|-- OBF
         x-|9        16|-- IBF
         x-|10       15|-- IRQB
         x-|11       14|-- LED_CS
     GND --|12       13|-x
           +-----------+
```

### Pin Connections

| Pin | Name | Signal | To |
|-----|------|--------|----|
| 1 | CLK | CLK_OUT | U1-19, U2-11 |
| 2 | I | MODE | J3-2 |
| 3 | I | A0 | J4-9 |
| 4 | I | SIG1 | J4-10 |
| 5 | I | IBF_CLRB | U1-17 |
| 6 | I | SIG2 | J4-12 |
| 7 | I | CSB | J4-11 |
| 8 | I | AUXB | U1-2 |
| 9 | I | nc | GND |
| 10 | I | nc | GND |
| 11 | I | nc | GND |
| 12 | GND | GND | GND |
| 13 | I | nc | GND |
| 14 | I/O | LED_CS | R6 to D1 anode |
| 15 | I/O | IRQB | J4-13 |
| 16 | I/O | IBF | U1-3, R8 to D3 anode |
| 17 | I/O | OBF | R7 to D2 anode |
| 18 | I/O | HDATA1 | J4-2, U2-18, U3-4 |
| 19 | I/O | HDATA0 | J4-1, U2-19, U3-3 |
| 20 | I/O | LE_IN | U3-11 |
| 21 | I/O | HDATA5 | J4-6, U2-14, U3-14 |
| 22 | I/O | LED_STATUS | R9 to D4 anode |
| 23 | I/O | RD_DATAB | U2-1 |
| 24 | VCC | +5V | +5V |

**C7 (100nF), C8 (10nF):** Pin 24 to Pin 12


---

## J1: PS/2 Port 1 Connector (Mini-DIN-6)

Female, front view:
```
       5   6
      3     4
        1 2
```

| Pin | Signal | To |
|-----|--------|----|
| 1 | PS2_DATA1 | U1-12, R3 to +5V |
| 3 | GND | GND |
| 4 | +5V | +5V |
| 5 | PS2_CLK1 | U1-13, R2 to +5V |

---

## J2: PS/2 Port 2 Connector (Mini-DIN-6)

Female, front view:
```
       5   6
      3     4
        1 2
```

| Pin | Signal | To |
|-----|--------|----|
| 1 | PS2_DATA2 | U1-10, R5 to +5V |
| 3 | GND | GND |
| 4 | +5V | +5V |
| 5 | PS2_CLK2 | U1-11, R4 to +5V |

---

## J3: Mode Select (1x3 Header)

| Pin | Signal | To |
|-----|--------|----|
| 1 | GND | GND |
| 2 | MODE | U4-2 |
| 3 | +5V | R11 (10K) to +5V |

**Jumper Position:**
- Pins 1-2: 6502 mode (MODE = LOW)
- Pins 2-3: Z80 mode (MODE = HIGH via R11)

---

## J4: Host Connector (1x15 Header)

```
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |1|2|3|4|5|6|7|8|9|A|B|C|D|E|F|
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   D D D D D D D D A S C S I + G
   0 1 2 3 4 5 6 7 0 1 S 2 R 5 N
                       B   Q V D
                           B
```

| Pin | Signal | 6502 | Z80 | To |
|-----|--------|------|-----|-----|
| 1 | D0 | D0 | D0 | HDATA0: U2-19, U3-3, U4-19 |
| 2 | D1 | D1 | D1 | HDATA1: U2-18, U3-4, U4-18 |
| 3 | D2 | D2 | D2 | HDATA2: U2-17, U3-7 |
| 4 | D3 | D3 | D3 | HDATA3: U2-16, U3-8 |
| 5 | D4 | D4 | D4 | HDATA4: U2-15, U3-13 |
| 6 | D5 | D5 | D5 | HDATA5: U2-14, U3-14, U4-21 |
| 7 | D6 | D6 | D6 | HDATA6: U2-13, U3-17 |
| 8 | D7 | D7 | D7 | HDATA7: U2-12, U3-18 |
| 9 | A0 | A0 | A0 | U4-3, U1-4 via R1 |
| 10 | SIG1 | R/WB | RDB | U4-4 |
| 11 | CSB | CSB | CSB | U4-7 |
| 12 | SIG2 | Φ2 | WRB | U4-6 |
| 13 | IRQB | IRQB | INTB | U4-15 |
| 14 | +5V | +5V | +5V | +5V |
| 15 | GND | GND | GND | GND |

---

## J5: ICSP Header (1x6)

Standard Microchip ICSP pinout:

```
  +-+-+-+-+-+-+
  |1|2|3|4|5|6|
  +-+-+-+-+-+-+
   M V G P P n
   C D S G G c
   L D S D C
   R
```

| Pin | Signal | To |
|-----|--------|----|
| 1 | MCLR/VPP | nc (active LVP mode) |
| 2 | VDD | +5V |
| 3 | VSS | GND |
| 4 | ICSPDAT | U1-19 (RA0/CLK_OUT) |
| 5 | ICSPCLK | U1-18 (RA1/OE_INB) |
| 6 | nc | - |

**Note:** The PIC uses Low-Voltage Programming (LVP) mode with MCLR disabled. RA3 is repurposed as the A0 input. Pin 1 of the ICSP header is not connected; programming uses only ICSPDAT/ICSPCLK. Do not use high-voltage programming mode.

---

## LEDs: Status Indicators

| LED | Pin | Signal | Color | Indicates |
|-----|-----|--------|-------|-----------|
| D1 | U4-14 | LED_CS | Green | Chip selected (active bus cycle) |
| D2 | U4-17 | OBF | Red | PIC has data waiting for host |
| D3 | U4-16 | IBF | Red | Host has sent command to PIC |
| D4 | U4-22 | LED_STATUS | Green | Host is polling status register |

---

## BOM (4 ICs + Support)

### ICs
| Ref | Part | Package | Function |
|-----|------|---------|----------|
| U1 | PIC16F18344-I/P | DIP-20 | Controller (dual PS/2) |
| U2 | 74HCT574 | DIP-20 | Output Latch |
| U3 | 74HCT373 | DIP-20 | Input Latch |
| U4 | ATF22V10C-xxPU | DIP-24 | GAL (Decoder/Status/Flags) |

### Passive Components
| Ref | Value | Package | Function |
|-----|-------|---------|----------|
| C1, C3, C5, C7 | 100nF | Ceramic | Decoupling (×4) |
| C2, C4, C6, C8 | 10nF | Ceramic | Decoupling (×4) |
| R1 | 1K | 1/4W | A0 series resistor |
| R2, R3 | 4.7K | 1/4W | PS/2 Port 1 pull-ups |
| R4, R5 | 4.7K | 1/4W | PS/2 Port 2 pull-ups |
| R6-R9 | 220Ω | 1/4W | LED current limit (×4) |
| R11 | 10K | 1/4W | MODE pull-up |

### Connectors & Indicators
| Ref | Part | Function |
|-----|------|----------|
| J1 | Mini-DIN-6 female | PS/2 Port 1 |
| J2 | Mini-DIN-6 female | PS/2 Port 2 |
| J3 | 1×3 header + jumper | Mode select |
| J4 | 1×15 header | Host bus |
| J5 | 1×6 header | ICSP |
| D1 | 3mm LED (green) | Chip select |
| D2 | 3mm LED (red) | OBF |
| D3 | 3mm LED (red) | IBF |
| D4 | 3mm LED (green) | Status read |

---

## Data Flow

### PS/2 activity (Port 1):
1. PIC sets AUXB = LOW (data from Port 1)
2. PIC disables U3 outputs (OE_INB=HIGH)
3. PIC drives IDATA0-7 with scan code
4. PIC pulses CLK_OUT HIGH
5. Rising edge latches into U2, sets OBF (in GAL), asserts IRQB
6. Host reads data port → RD_DATAB clears OBF (async reset in GAL)

### PS/2 activity (Port 2):
1. PIC sets AUXB = HIGH (data from Port 2)
2. PIC disables U3 outputs (OE_INB=HIGH)
3. PIC drives IDATA0-7 with scan code
4. PIC pulses CLK_OUT HIGH
5. Rising edge latches into U2, sets OBF (in GAL), asserts IRQB
6. Host reads status → sees AUXB=1 (bit 5), knows data is from Port 2
7. Host reads data port → RD_DATAB clears OBF (async reset in GAL)

### Host sends command (6502 mode):
1. Host drives D0-D7 with command, sets A0, R/WB=LOW
2. During Φ2 HIGH with R/WB=LOW: write in progress, IBF set (combinatorial)
3. LE_IN follows R/WB (LOW during write = latched)
4. PIC sees IBF=HIGH, enables U3 (OE_INB=LOW), reads IDATA
5. PIC pulses IBF_CLRB LOW to clear IBF

### Host sends command (Z80 mode):
1. Host drives D0-D7 with command, WRB goes LOW
2. LE_IN = !WRB = HIGH (transparent), IBF set (combinatorial)
3. When WRB rises: LE_IN goes LOW (data latched)
4. PIC sees IBF=HIGH, enables U3 (OE_INB=LOW), reads IDATA
5. PIC pulses IBF_CLRB LOW to clear IBF

### Host reads status (both modes):
1. Host performs read with A0=1
2. LED_STATUS enables GAL tri-state outputs on HDATA0, HDATA1, HDATA5
3. GAL drives OBF→D0, IBF→D1, AUXB→D5
