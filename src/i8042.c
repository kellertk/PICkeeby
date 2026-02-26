/**
 * PICkeeby - i8042.c
 * i8042 keyboard controller emulation
 */

#include <xc.h>
#include "i8042.h"

// Initialize EEPROM addresses 1-31 (i8042 internal RAM mapping)
// Address 0 is command byte (kept in RAM, not EEPROM)
// Using 0x00 instead of 0xFF ensures EEPROM is explicitly programmed
__EEPROM_DATA(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);  // 1-8
__EEPROM_DATA(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);  // 9-16
__EEPROM_DATA(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);  // 17-24
__EEPROM_DATA(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);  // 25-32
#include "ps2.h"
#include "host.h"
#include "pins.h"

// Command byte bits
#define CMD_KBD_INT     0x01
#define CMD_AUX_INT     0x02
#define CMD_SYS_FLAG    0x04
#define CMD_KBD_DIS     0x10
#define CMD_AUX_DIS     0x20
#define CMD_XLAT        0x40

// i8042 controller commands (written with A0=1)
#define CMD_READ_CMD    0x20
#define CMD_WRITE_CMD   0x60
#define CMD_AUX_DIS_CMD 0xA7
#define CMD_AUX_EN_CMD  0xA8
#define CMD_AUX_TEST    0xA9
#define CMD_SELF_TEST   0xAA
#define CMD_KBD_TEST    0xAB
#define CMD_KBD_DIS_CMD 0xAD
#define CMD_KBD_EN_CMD  0xAE
#define CMD_READ_INPUT  0xC0
#define CMD_READ_OUT    0xD0
#define CMD_WRITE_OUT   0xD1
#define CMD_WRITE_KBD   0xD2
#define CMD_WRITE_AUX   0xD3
#define CMD_WRITE_AUX_DEV 0xD4
#define CMD_PULSE_RESET 0xFE

// Pending command states
typedef enum {
    PEND_NONE = 0,
    PEND_CMD_BYTE,      // Next byte sets command byte
    PEND_OUTPUT_PORT,   // 0xD1: write output port (ignored, no A20/reset)
    PEND_KBD_OUTPUT,    // 0xD2: write keyboard output buffer
    PEND_AUX_OUTPUT,    // 0xD3: write aux output buffer
    PEND_AUX_DEVICE,    // 0xD4: write to auxiliary device
    PEND_EEPROM         // 0x61-0x7F: write to EEPROM
} pending_t;

static uint8_t command_byte;
static pending_t pending_cmd;
static uint8_t eeprom_addr;

static void process_command(uint8_t cmd) {
    switch (cmd) {
        case CMD_READ_CMD:
            host_send_data(command_byte, false);
            break;

        case CMD_WRITE_CMD:
            pending_cmd = PEND_CMD_BYTE;
            break;

        case CMD_AUX_DIS_CMD:
            command_byte |= CMD_AUX_DIS;
            ps2_inhibit(PS2_PORT2, true);
            break;

        case CMD_AUX_EN_CMD:
            command_byte &= ~CMD_AUX_DIS;
            ps2_inhibit(PS2_PORT2, false);
            break;

        case CMD_AUX_TEST:
            host_send_data(0x00, false);  // Pass
            break;

        case CMD_SELF_TEST:
            host_send_data(0x55, false);  // Pass
            break;

        case CMD_KBD_TEST:
            host_send_data(0x00, false);  // Pass
            break;

        case CMD_KBD_DIS_CMD:
            command_byte |= CMD_KBD_DIS;
            ps2_inhibit(PS2_PORT1, true);
            break;

        case CMD_KBD_EN_CMD:
            command_byte &= ~CMD_KBD_DIS;
            ps2_inhibit(PS2_PORT1, false);
            break;

        case CMD_READ_INPUT:
            host_send_data(0x00, false);
            break;

        case CMD_READ_OUT:
            host_send_data(0xCF, false);  // A20 high, no reset
            break;

        case CMD_WRITE_OUT:
            pending_cmd = PEND_OUTPUT_PORT;
            break;

        case CMD_WRITE_KBD:
            pending_cmd = PEND_KBD_OUTPUT;
            break;

        case CMD_WRITE_AUX:
            pending_cmd = PEND_AUX_OUTPUT;
            break;

        case CMD_WRITE_AUX_DEV:
            pending_cmd = PEND_AUX_DEVICE;
            break;

        case CMD_PULSE_RESET:
            RESET();
            break;

        default:
            // Handle internal RAM range mapped to EEPROM
            // 0x20-0x3F: read (0x20 = command byte, 0x21-0x3F = EEPROM[1-31])
            // 0x60-0x7F: write (0x60 = command byte, 0x61-0x7F = EEPROM[1-31])
            if (cmd >= 0x20 && cmd <= 0x3F) {
                uint8_t addr = cmd & 0x1F;
                if (addr == 0) {
                    host_send_data(command_byte, false);
                } else {
                    host_send_data(eeprom_read(addr), false);
                }
            } else if (cmd >= 0x60 && cmd <= 0x7F) {
                uint8_t addr = cmd & 0x1F;
                if (addr == 0) {
                    pending_cmd = PEND_CMD_BYTE;
                } else {
                    eeprom_addr = addr;
                    pending_cmd = PEND_EEPROM;
                }
            }
            break;
    }
}

static void process_data(uint8_t data) {
    switch (pending_cmd) {
        case PEND_CMD_BYTE:
            command_byte = data;
            if (data & CMD_KBD_DIS) {
                ps2_inhibit(PS2_PORT1, true);
            } else {
                ps2_inhibit(PS2_PORT1, false);
            }
            if (data & CMD_AUX_DIS) {
                ps2_inhibit(PS2_PORT2, true);
            } else {
                ps2_inhibit(PS2_PORT2, false);
            }
            break;

        case PEND_OUTPUT_PORT:
            // Ignore A20 and reset bits
            break;

        case PEND_KBD_OUTPUT:
            host_send_data(data, false);
            break;

        case PEND_AUX_OUTPUT:
            host_send_data(data, true);
            break;

        case PEND_AUX_DEVICE:
            ps2_tx_write(PS2_PORT2, data);
            break;

        case PEND_EEPROM:
            eeprom_write(eeprom_addr, data);
            break;

        case PEND_NONE:
        default:
            // Data for keyboard
            ps2_tx_write(PS2_PORT1, data);
            break;
    }
    pending_cmd = PEND_NONE;
}

void i8042_init(void) {
    command_byte = CMD_SYS_FLAG | CMD_XLAT;
    pending_cmd = PEND_NONE;
    ps2_inhibit(PS2_PORT1, false);
    ps2_inhibit(PS2_PORT2, false);
}

void i8042_process_byte(uint8_t byte, bool is_command) {
    if (pending_cmd != PEND_NONE) {
        process_data(byte);
    } else if (is_command) {
        process_command(byte);
    } else {
        // Data for keyboard
        ps2_tx_write(PS2_PORT1, byte);
    }
}

void i8042_task(void) {
    // Forward keyboard (Port 1) data to host
    if (ps2_rx_available(PS2_PORT1)) {
        uint8_t data = ps2_rx_read(PS2_PORT1);
        host_send_data(data, false);
    }

    // Forward auxiliary (Port 2) data to host
    if (ps2_rx_available(PS2_PORT2)) {
        uint8_t data = ps2_rx_read(PS2_PORT2);
        host_send_data(data, true);
    }
}
