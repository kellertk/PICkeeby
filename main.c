/**
 * @file main.c
 * @author kellertk
 * A PIC16F716-based PS/2 keyboard to ASCII shift register interface
 * This is proobably adaptable to other PIC devices with minimal changes
 * Only works with US layout keyboards
 */

#include <builtins.h>
#pragma config FOSC = HS        // Oscillator Selection bits
#pragma config WDTE = OFF       // Watchdog Timer Disable
#pragma config PWRTE = ON       // Power-up Timer Enable bit
#pragma config BOREN = ON       // Brown-out Reset Enable bit
#pragma config BODENV = 40      // Brown-out Reset Voltage bit
#pragma config CP = OFF         // Code Protect

#define _XTAL_FREQ 20000000     // 20 MHz

#include <pic.h>
#include <pic16f716.h>

#define KBD_CLOCK      PORTBbits.RB0
#define KBD_DATA       PORTBbits.RB4
#define SR_CLK         PORTBbits.RB6
#define SR_DATA        PORTBbits.RB5
#define INTB           PORTBbits.RB7
#define KBD_CLOCK_DIR  TRISBbits.TRISB0
#define KBD_DATA_DIR   TRISBbits.TRISB4
#define SR_CLK_DIR     TRISBbits.TRISB6
#define SR_DATA_DIR    TRISBbits.TRISB5
#define INTB_DIR       TRISBbits.TRISB7
#define DEBUG_LED      PORTAbits.RA2
#define DEBUG_LED_DIR  TRISAbits.TRISA2

#include <xc.h>
#include "keymap.h"

// Circular buffer - 16 bytes
#define BUFFER_SIZE 16
#define BUFFER_MASK 0x0F
volatile uint8_t keyBuffer[BUFFER_SIZE];
volatile uint8_t bufferHead = 0;
volatile uint8_t bufferTail = 0;

// Store received data in circular buffer
void decodeScancode(uint8_t data) {
    int c = getkbdchar(data);
    if (c == -1) return;

    // Buffer first byte
    uint8_t nextHead = (bufferHead + 1) & BUFFER_MASK;
    if (nextHead != bufferTail) {
        keyBuffer[bufferHead] = (uint8_t)c;
        bufferHead = nextHead;

        // Check if there's a second byte buffered
        if (hasUTF8Buffered()) {
            c = getkbdchar(0);  // Get buffered byte
            if (c != -1) {
                nextHead = (bufferHead + 1) & BUFFER_MASK;
                if (nextHead != bufferTail) {
                    keyBuffer[bufferHead] = (uint8_t)c;
                    bufferHead = nextHead;
                }
            }
        }
    }
}

void __interrupt() isr(void) {
    // PS/2 state machine
    static uint8_t ps2_data = 0;
    static uint8_t ps2_state = 0; // bits 0-3: count, bit 4: parity

    // Handle PS/2 clock interrupt
    if (INTCONbits.INTF) {
        INTCONbits.INTF = 0;
        TMR0 = 22;                // Preload for ~3ms timeout
        INTCONbits.TMR0IF = 0;    // Clear Timer0 overflow flag

        uint8_t bit = KBD_DATA & 1;
        uint8_t count = ps2_state & 0x0F;

        switch(count) {
            case 0:               // Start bit - must be 0
                if (!bit) {
                    ps2_state = 1;
                    ps2_data = 0; // Clear data for new packet
                }
                break;
            case 9:               // Parity bit
                if ((((ps2_state >> 4)) & 1) != bit) {
                    // Parity OK, increment state
                    ps2_state = (ps2_state & 0xF0) | 10;
                } else {         // Parity error, reset
                    ps2_state = 0;
                }
                break;
            case 10:             // Stop bit - must be 1
                DEBUG_LED = 1;
                if (bit) decodeScancode(ps2_data);
                ps2_state = 0;
                break;
            default:             // count 1-8: data bits
                ps2_data >>= 1;
                if (bit) {
                    ps2_data |= 0x80;
                    ps2_state ^= 0x10;
                }
                ps2_state = (ps2_state & 0xF0) | ((count + 1) & 0x0F);
                break;
        }
    }

    // Handle Timer0 timeout - reset PS/2 packet state if no clock for 3ms
    if (INTCONbits.TMR0IF) {
        ps2_state = 0;
        ps2_data = 0;
        DEBUG_LED = 0;
        INTCONbits.TMR0IF = 0;
        TMR0 = 22;
    }
}

void setup(void) {
    TRISA = 0xFF;
    TRISB = 0xFF;       // all ports are input
    ADCON1 = 0b110;     // set all pins to digital I/O
    SR_CLK_DIR = 0;     // output
    SR_DATA_DIR = 0;    // output
    DEBUG_LED_DIR = 0;  // output

    SR_CLK = 0;
    SR_DATA = 0;
    DEBUG_LED = 0;
    DEBUG_LED = 1;
    __delay_ms(10);
    DEBUG_LED = 0;


    // Configure Timer0 for PS/2 timeout detection (~3ms)
    // Prescaler 1:64, gives 12.8μs per tick at 20MHz
    OPTION_REGbits.T0CS = 0;    // Timer mode (internal clock)
    OPTION_REGbits.PSA = 0;     // Assign prescaler to Timer0
    OPTION_REGbits.PS = 0b101;  // Prescaler 1:64
    TMR0 = 22;                  // Preload for ~3ms timeout (234 ticks)
    INTCONbits.TMR0IF = 0;      // Clear overflow flag
    INTCONbits.TMR0IE = 1;      // Enable Timer0 interrupt

    // Enable external interrupt on RB0/INT (KBD_CLOCK) for data sampling
    OPTION_REGbits.INTEDG = 0;  // Interrupt on falling edge
    INTCONbits.INTF = 0;        // Clear interrupt flag
    INTCONbits.INTE = 1;        // Enable external interrupt

    INTCONbits.GIE = 1;         // Enable the interrupt vector
}

void shiftOutByte(uint8_t data) {
    // Target ~29ms latency (~34 cps)
    // Faster shifts are usually possible but also usually not needed
    for (uint8_t i = 0; i < 8; i++) {
        SR_DATA = (data >> (7 - i)) & 1; // Set data bit
        __delay_us(100);                 // Setup time
        SR_CLK = 1;
        __delay_us(3500);                // Hold time
        SR_CLK = 0;
        __delay_us(100);                 // Recovery time
    }
    SR_DATA = 0;  // Reset data line to low
}

int main() {
    setup();

    while (1) {
        // Process buffer if data available
        if (bufferHead != bufferTail) {
            // Check if MCU is ready to receive (INTB high)
            if (INTB) {
                uint8_t data = keyBuffer[bufferTail];
                bufferTail = (bufferTail + 1) & BUFFER_MASK;
                shiftOutByte(data);
            }
        }
    }
    return 0;
}
