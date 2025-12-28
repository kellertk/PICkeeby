#include <pic.h>
#include <xc.h>
#include "ps2_send.h"

#define _XTAL_FREQ 20000000

// Pin definitions (must match main.c)
#define KBD_CLOCK      PORTBbits.RB0
#define KBD_DATA       PORTBbits.RB4
#define KBD_CLOCK_DIR  TRISBbits.TRISB0
#define KBD_DATA_DIR   TRISBbits.TRISB4

// Command queue - stores command ID and optional data
#define CMD_BUFFER_SIZE 8
#define CMD_BUFFER_MASK 0x07

// Command IDs
#define CMD_SET_LEDS      0xED
#define CMD_ECHO          0xEE
#define CMD_SET_TYPEMATIC 0xF3
#define CMD_ENABLE        0xF4
#define CMD_DISABLE       0xF5
#define CMD_SET_DEFAULTS  0xF6
#define CMD_RESET         0xFF

typedef struct {
    uint8_t cmd;   // Command byte
    uint8_t data;  // Data byte (if needed)
} PS2Command;

static volatile PS2Command cmdBuffer[CMD_BUFFER_SIZE];
static volatile uint8_t cmdHead = 0;
static volatile uint8_t cmdTail = 0;

// Transmission timeout flag (set by Timer0 ISR in main.c)
volatile uint8_t tx_timeout = 0;

// Echo monitoring flag (set by Timer1 ISR in main.c)
volatile uint8_t echo_timeout = 0;

// Echo tracking
static uint8_t echo_pending = 0;
static uint8_t echo_failures = 0;

// Queue a command structure
#pragma warning push
#pragma warning disable 1510
static void queueCommand(uint8_t cmd, uint8_t data) {
    uint8_t nextHead = (cmdHead + 1) & CMD_BUFFER_MASK;
    if (nextHead != cmdTail) {
        cmdBuffer[cmdHead].cmd = cmd;
        cmdBuffer[cmdHead].data = data;
        cmdHead = nextHead;
    }
}
#pragma warning pop

void ps2_setLEDs(uint8_t leds) {
    queueCommand(CMD_SET_LEDS, leds);
}
void ps2_echo(void) {
    queueCommand(CMD_ECHO, 0);
    echo_pending = 1;
}
void ps2_setTypematic(uint8_t rate) {
    queueCommand(CMD_SET_TYPEMATIC, rate);
}
void ps2_enable(void) {
    queueCommand(CMD_ENABLE, 0);
}
void ps2_disable(void) {
    queueCommand(CMD_DISABLE, 0);
}
void ps2_setDefaults(void) {
    queueCommand(CMD_SET_DEFAULTS, 0);
}

void ps2_reset(void) {
    queueCommand(CMD_RESET, 0);
    echo_pending = 0;
    echo_failures = 0;
}

void ps2_initKeyboard(void) {
    // Turn off all LEDs initially
    ps2_setLEDs(0x00);

    // Enable numlock LED
    ps2_setLEDs(0x02);

    // Set typematic: 500ms delay, 30 reports/sec
    // Bits 6-5: 01 = 500ms delay
    // Bits 4-0: 00000 = 30 Hz
    ps2_setTypematic(0x20);

    // Enable keyboard scanning
    ps2_enable();
}

static uint8_t ps2_sendByte(uint8_t data) {
    uint8_t parity = 1;  // Odd parity starts at 1
    uint8_t response = 0;

    INTCONbits.INTE = 0;

    // Request to send sequence
    KBD_CLOCK_DIR = 0;  // Output
    KBD_CLOCK = 0;
    __delay_us(120);
    KBD_DATA_DIR = 0;   // Output
    KBD_DATA = 0;
    KBD_CLOCK_DIR = 1;  // Input

    // Wait for device to bring Clock low
    tx_timeout = 0;
    TMR0 = 22;
    INTCONbits.TMR0IF = 0;
    while (KBD_CLOCK && !tx_timeout);
    if (tx_timeout) goto cleanup;

    // Send 8 data bits
    for (uint8_t i = 0; i < 8; i++) {
        uint8_t bit = (data >> i) & 1;
        KBD_DATA = bit;                     // Setup data
        if (bit) parity ^= 1;

        while (!KBD_CLOCK && !tx_timeout);  // Wait for Clock high
        if (tx_timeout) goto cleanup;
        TMR0 = 22;
        INTCONbits.TMR0IF = 0;

        while (KBD_CLOCK && !tx_timeout);   // Wait for Clock low
        if (tx_timeout) goto cleanup;
    }

    KBD_DATA = parity;                      // Setup parity bit
    while (!KBD_CLOCK && !tx_timeout);      // Wait high
    if (tx_timeout) goto cleanup;
    TMR0 = 22;
    INTCONbits.TMR0IF = 0;
    while (KBD_CLOCK && !tx_timeout);       // Wait low
    if (tx_timeout) goto cleanup;

    // Release Data line
    KBD_DATA_DIR = 1;  // Input

    // Wait for device ACK (Data low)
    while (!KBD_CLOCK && !tx_timeout);      // Wait high
    if (tx_timeout) goto cleanup;
    TMR0 = 22;
    INTCONbits.TMR0IF = 0;
    while (KBD_CLOCK && !tx_timeout);       // Wait low
    if (tx_timeout) goto cleanup;

    // Wait for device to release Data and Clock
    while ((!KBD_CLOCK || !KBD_DATA) && !tx_timeout);
    if (tx_timeout) goto cleanup;

    // Receive response byte
    tx_timeout = 0;
    TMR0 = 22;
    INTCONbits.TMR0IF = 0;

    while (KBD_CLOCK && !tx_timeout);          // Wait for Clock low
    if (tx_timeout || KBD_DATA) goto cleanup;  // Start bit must be 0

    // Read 8 data bits
    for (uint8_t i = 0; i < 8; i++) {
        while (!KBD_CLOCK && !tx_timeout);     // Wait Clock high
        if (tx_timeout) goto cleanup;
        while (KBD_CLOCK && !tx_timeout);      // Wait Clock low
        if (tx_timeout) goto cleanup;
        TMR0 = 22;
        INTCONbits.TMR0IF = 0;

        response |= ((KBD_DATA & 1) << i);
    }

    // Read parity bit
    while (!KBD_CLOCK && !tx_timeout);         // Wait Clock high
    if (tx_timeout) goto cleanup;
    while (KBD_CLOCK && !tx_timeout);          // Wait Clock low
    if (tx_timeout) goto cleanup;

    // Read stop bit
    while (!KBD_CLOCK && !tx_timeout);         // Wait Clock high
    if (tx_timeout) goto cleanup;
    while (KBD_CLOCK && !tx_timeout);          // Wait Clock low

cleanup:
    // Restore pins to input mode
    KBD_CLOCK_DIR = 1;
    KBD_DATA_DIR = 1;

    // Re-enable kb_clock interrupt
    INTCONbits.INTF = 0;
    INTCONbits.INTE = 1;

    return tx_timeout ? 0xFF : response;
}

void ps2_processCommands(void) {
    static uint8_t retry_count = 0;
    static uint8_t state = 0;  // 0=send_cmd, 1=send_data

    // Check for echo timeout
    if (echo_timeout) {
        echo_timeout = 0;
        if (!echo_pending) {
            ps2_echo();
        }
    }

    if (cmdHead != cmdTail) {
        PS2Command* current = (PS2Command*)&cmdBuffer[cmdTail];

        if (state == 0) {
            // Send command byte
            uint8_t response = ps2_sendByte(current->cmd);

            if (response == 0xFE && retry_count < 2) {
                // Resend request - retry from start
                retry_count++;
                return;
            } else if (response != 0xFA && response != 0xEE) {
                // Error or timeout - handle based on command
                if (current->cmd == CMD_ECHO) {
                    echo_pending = 0;
                    echo_failures++;
                    if (echo_failures >= 3) {
                        ps2_reset();
                    }
                }
                cmdTail = (cmdTail + 1) & CMD_BUFFER_MASK;
                retry_count = 0;
                state = 0;
                return;
            }

            // Handle echo response
            if (current->cmd == CMD_ECHO && response == 0xEE) {
                echo_pending = 0;
                echo_failures = 0;
            }

            // Command ACKed - check if we need to send data
            if (current->cmd == CMD_SET_LEDS || current->cmd == CMD_SET_TYPEMATIC) {
                state = 1;  // Need to send data byte
                return;
            }

            // Single-byte command complete
            cmdTail = (cmdTail + 1) & CMD_BUFFER_MASK;
            retry_count = 0;
            state = 0;
        } else {
            // Send data byte
            uint8_t response = ps2_sendByte(current->data);

            if (response == 0xFE && retry_count < 2) {
                // Resend entire command+data
                retry_count++;
                state = 0;  // Restart from command byte
                return;
            } else if (response != 0xFA) {
                // Error - skip command
                cmdTail = (cmdTail + 1) & CMD_BUFFER_MASK;
                retry_count = 0;
                state = 0;
                return;
            }

            // Data byte ACKed - command complete
            cmdTail = (cmdTail + 1) & CMD_BUFFER_MASK;
            retry_count = 0;
            state = 0;
        }
    }
}
