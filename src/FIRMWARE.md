# PICkeeby Firmware Design

## Overview

PICkeeby emulates an i8042 keyboard controller, bridging PS/2 devices to a host computer's I/O bus. The firmware runs on a PIC16F18344 at 32 MHz.

```
+-------------+       +-------------+       +-------------+
|   PS/2      |       |             |       |    Host     |
|  Keyboard   |<----->|   PICkeeby  |<----->|   Computer  |
|   / Mouse   |       |             |       |  (6502/Z80) |
+-------------+       +-------------+       +-------------+
     Port 1/2           PIC16F18344          via latches
```

## Architecture

The firmware is organized into four modules:

| Module | File | Purpose |
|--------|------|---------|
| PS/2 Protocol | ps2.c/h | Bidirectional PS/2 communication |
| Host Interface | host.c/h | Latch control and data transfer |
| i8042 Emulation | i8042.c/h | Command processing and port management |
| Main | main.c | Initialization and main loop |

### Data Flow

**Device → Host (scan codes):**
```
PS/2 Device → ps2_clock_isr → rx_queue → i8042_task → host_send_data → output_queue → host_pump_output → 74HCT574 → Host
```

**Host → Device (commands):**
```
Host → 74HCT373 → IBF → host_read_input → i8042_process_byte → ps2_tx_write → tx_queue → ps2_task → PS/2 Device
```

---

## Module Details

### ps2.c — PS/2 Protocol Layer

Implements the PS/2 protocol using bit-banged I/O with interrupt-driven clock handling.

**State Machine:**
```
                    start bit (DATA=0)
        IDLE ─────────────────────────────> RX
          │                                  │
          │                                  │ 11 clocks
          │                                  ▼
          │<─────────────────────────────────┘
          │
          │  ps2_task() initiates TX
          ▼
         TX ──────────────────────────────> TX_ACK
                   11 clocks                  │
                                              │ device ACK
                                              ▼
                                            IDLE
```

**Receive (device-initiated):**
1. Device pulls DATA low (start bit)
2. ISR detects start, enters RX state
3. 8 data bits shifted in on falling clock edges
4. Parity bit checked
5. Stop bit validated
6. Byte queued if valid

**Transmit (host-initiated):**
1. `ps2_task()` detects pending TX data
2. PIC holds CLK low for 150µs (request-to-send)
3. PIC pulls DATA low (start bit)
4. PIC releases CLK, device clocks out bits
5. ISR sends 8 data bits + parity + stop
6. Device sends ACK (pulls DATA low)

**Functions:**
- `ps2_clock_isr(port)` — Called from ISR on clock falling edge
- `ps2_task()` — Initiates pending transmissions, checks for timeouts
- `ps2_inhibit(port, inhibit)` — Enable/disable port communication

**Timeout Recovery:**
If no clock activity occurs for ~2ms while in RX or TX state, the port resets to IDLE. This prevents permanent data misalignment from dropped clock pulses.

### host.c — Host Bus Interface

Manages the 74HCT574 output latch and 74HCT373 input latch.

**A0 Latching:**
The PIC's CLC1 module is configured as a D flip-flop that latches the A0 signal on the rising edge of IBF. This allows the firmware to distinguish command register writes (A0=1) from data register writes (A0=0).

```
CLC1 Configuration:
  Mode: D flip-flop with R
  D input: RA3 (A0 from host)
  CLK input: RA4 (IBF from GAL)
  Output: CLC1OUT register bit
```

**Output Flow (PIC → Host):**
```
1. Set AUXB (port identifier)
2. Drive IDATA bus with data byte
3. Pulse CLK_OUT high → latches data, sets OBF in GAL
4. Host reads data register (A0=0) → clears OBF
```

**Input Flow (Host → PIC):**
```
1. Host writes to data or command register → GAL sets IBF, CLC latches A0
2. PIC polls IBF_GetValue()
3. When IBF=1: read latched A0 from CLC1OUT
4. Switch IDATA to input, enable latch outputs
5. Read byte from IDATA_PORT
6. Disable latch outputs, restore IDATA to output
7. Pulse IBF_CLRB low → clears IBF
```

**Functions:**
- `host_init()` — Initialize latches and CLC1 for A0 latching
- `host_send_data(data, aux)` — Queue byte for host
- `host_pump_output()` — Transfer one queued byte to output latch
- `host_ibf_active()` — Check if host has written data
- `host_get_a0()` — Read latched A0 value from CLC1
- `host_read_input()` — Read and acknowledge host write

### i8042.c — Controller Emulation

Emulates i8042 command set (without A20 gate or system reset).

**Command Byte (internal register, read via 0x20, write via 0x60):**
```
Bit 6: XLAT    - Scan code translation (stored but not implemented)
Bit 5: AUX_DIS - Disable auxiliary port (mouse)
Bit 4: KBD_DIS - Disable keyboard port
Bit 2: SYS     - System flag
Bit 1: AUX_INT - Auxiliary interrupt enable
Bit 0: KBD_INT - Keyboard interrupt enable
```

**Supported Commands:**

| Command | Function |
|---------|----------|
| 0x20 | Read command byte |
| 0x21-0x3F | Read EEPROM[1-31] |
| 0x60 | Write command byte |
| 0x61-0x7F | Write EEPROM[1-31] |
| 0xA7 | Disable auxiliary port |
| 0xA8 | Enable auxiliary port |
| 0xA9 | Test auxiliary port (returns 0x00) |
| 0xAA | Controller self-test (returns 0x55) |
| 0xAB | Test keyboard port (returns 0x00) |
| 0xAD | Disable keyboard port |
| 0xAE | Enable keyboard port |
| 0xC0 | Read input port |
| 0xD0 | Read output port |
| 0xD1 | Write output port (ignored) |
| 0xD2 | Write keyboard output buffer |
| 0xD3 | Write auxiliary output buffer |
| 0xD4 | Write to auxiliary device |
| 0xFE | Reset keyboard controller |

**Command vs Data Routing:**

The hardware provides only one write path (IBF), with no A0 distinction. Commands are identified by value:
- Bytes in command ranges (0x20-0x3F, 0x60-0x7F, 0xA7-0xAF, etc.) → processed as commands
- Other bytes → forwarded to keyboard via PS/2

Multi-byte commands use a pending state machine:
```c
typedef enum {
    PEND_NONE,
    PEND_CMD_BYTE,    // Next byte is command byte data
    PEND_OUTPUT_PORT, // Next byte is output port data
    PEND_KBD_OUTPUT,  // Next byte goes to keyboard output buffer
    PEND_AUX_OUTPUT,  // Next byte goes to aux output buffer
    PEND_AUX_DEVICE,  // Next byte goes to aux device via PS/2
    PEND_EEPROM       // Next byte goes to EEPROM
} pending_t;
```

**EEPROM Storage:**
The i8042 internal RAM commands (0x20-0x3F read, 0x60-0x7F write) are mapped to the PIC's EEPROM. Address 0 remains the command byte; addresses 1-31 provide 31 bytes of nonvolatile storage accessible to the host.

### main.c

**Main Loop:**
```c
while (1) {
    ps2_task();        // Initiate pending PS/2 transmissions
    i8042_task();      // Forward PS/2 RX data to host queue

    if (host_data_pending()) {
        host_pump_output();  // Send one byte to output latch
    }

    if (host_ibf_active()) {
        bool is_cmd = host_get_a0();
        uint8_t byte = host_read_input();
        i8042_process_byte(byte, is_cmd);
    }
}
```

---

## Pin Assignments

| Pin | Port | Function | Direction |
|-----|------|----------|-----------|
| 19 | RA0 | CLK_OUT | Output |
| 18 | RA1 | OE_INB | Output |
| 17 | RA2 | IBF_CLRB | Output |
| 4 | RA3 | A0 | Input (CLC1 D input) |
| 3 | RA4 | IBF | Input (CLC1 CLK input) |
| 2 | RA5 | AUXB | Output |
| 13 | RB4 | PS2_CLK1 | Bidirectional (open-drain) |
| 12 | RB5 | PS2_DATA1 | Bidirectional (open-drain) |
| 11 | RB6 | PS2_CLK2 | Bidirectional (open-drain) |
| 10 | RB7 | PS2_DATA2 | Bidirectional (open-drain) |
| 16-9 | RC0-7 | IDATA[0-7] | Bidirectional |

---

## Interrupt Handling

Single ISR handles both PS/2 ports via interrupt-on-change:

```c
void __interrupt() ISR(void) {
    if (PIR0bits.IOCIF) {
        if (IOCBFbits.IOCBF4) {
            IOCBFbits.IOCBF4 = 0;
            ps2_clock_isr(PS2_PORT1);
        }
        if (IOCBFbits.IOCBF6) {
            IOCBFbits.IOCBF6 = 0;
            ps2_clock_isr(PS2_PORT2);
        }
    }
}
```

IOC triggers on falling edge of PS/2 clock (IOCBN = 0x50).

---

## Timing Considerations

**PS/2 Clock:** 10-16.7 kHz (60-100 µs per bit)
- ISR must complete within ~30 µs to catch next edge
- At 32 MHz, ~960 instruction cycles available

**Host Read Rate:** Depends on host software polling interval
- Output queue (32 entries) buffers scan codes from PS/2 devices
- If host doesn't read before queue fills, new data is dropped
- OBF flag in status register tells host when data is waiting
- To avoid output buffer overrun, the host should read data once every 12.5 ms on average
- The output buffer can hold approximately 500 ms of keystrokes from a fast typist before dropping data

**Host Write Rate:** Not critical
- PIC polls IBF in main loop, typically sub-millisecond response
- Host must wait for IBF to clear before writing next byte

**TX Request-to-Send:** 150 µs clock inhibit before transmission

---

## Limitations

1. **No scan code translation** — XLAT bit is stored but Set 2 → Set 1 translation is not implemented

2. **No A20 control** — 0xD1 output port writes are acknowledged but A20 gate bit is ignored

3. **Single-byte lookahead** — Multi-byte PS/2 sequences (e.g., extended keys) are interpreted by the host

4. **LVP programming only** — MCLR is disabled; do not use high-voltage programming mode
