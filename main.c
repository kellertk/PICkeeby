/**
 * @file main.c
 * @author kellertk
 * A PIC16F716-based PS/2 keyboard to ASCII shift register interface
 * This is proobably adaptable to other PIC devices with minimal changes
 * Only works with US layout keyboards
 */

#include <builtins.h>
#pragma config FOSC = HS        // Oscillator Selection bits
#pragma config WDTE = ON        // Watchdog Timer Enable bit
#pragma config PWRTE = OFF      // Power-up Timer Enable bit
#pragma config BOREN = ON       // Brown-out Reset Enable bit
#pragma config BODENV = 40      // Brown-out Reset Voltage bit
#pragma config CP = OFF         // Code Protect

#define _XTAL_FREQ 20000000 // 20 MHz

#include <pic.h>
#include <pic16f716.h>

#define KBD_CLOCK     PORTBbits.RB0
#define KBD_DATA      PORTBbits.RB4
#define SR_CLK        PORTBbits.RB6
#define SR_DATA       PORTBbits.RB5
#define INTB          PORTBbits.RB7
#define KBD_CLOCK_DIR TRISBbits.TRISB0
#define KBD_DATA_DIR  TRISBbits.TRISB4
#define SR_CLK_DIR    TRISBbits.TRISB6
#define SR_DATA_DIR   TRISBbits.TRISB5
#define INTB_DIR      TRISBbits.TRISB7
#define DEBUG_LED     PORTAbits.RA2
#define DEBUG_LED_DIR TRISAbits.TRISA2

#include <xc.h>
#include "keymap.h"

// Circular buffer - 16 bytes
#define BUFFER_SIZE 16
#define BUFFER_MASK 0x0F
volatile uint8_t keyBuffer[BUFFER_SIZE];
volatile uint8_t bufferIdx = 0; // bits 0-3: tail, bits 4-7: head

// Store received data in circular buffer (17 bytes total overhead)
void decodeScancode(uint8_t data) {
    int c = getkbdchar(data);
    if (c == -1) return;

    // Buffer first byte
    uint8_t head = (bufferIdx >> 4) & BUFFER_MASK;
    uint8_t tail = bufferIdx & BUFFER_MASK;
    uint8_t nextHead = (head + 1) & BUFFER_MASK;
    if (nextHead != tail) {
        keyBuffer[head] = (uint8_t)c;
        bufferIdx = (uint8_t)((nextHead << 4) | tail);

        // Check if there's a second UTF-8 byte buffered
        if (hasUTF8Buffered()) {
            c = getkbdchar(0);  // Get buffered byte
            if (c != -1) {
                head = (bufferIdx >> 4) & BUFFER_MASK;
                tail = bufferIdx & BUFFER_MASK;
                nextHead = (head + 1) & BUFFER_MASK;
                if (nextHead != tail) {
                    keyBuffer[head] = (uint8_t)c;
                    bufferIdx = (uint8_t)((nextHead << 4) | tail);
                }
            }
        }
    }
}

void __interrupt() ps2ISR(void) {
    // 1 start bit = 0
    // 8 data bits, LSB first
    // 1 parity bit (odd)
    // 1 stop bit = 1
    static uint8_t data = 0;
    static uint8_t state = 0; // bits 0-3: count, bit 4: parity

    uint8_t bit = KBD_DATA & 1;
    uint8_t count = state & 0x0F;

    if (count == 0 && bit) { INTCONbits.INTF = 0; return; } // Invalid start bit
    if (count == 0) {
        // Start bit detected, discard
        state = 1;
        INTCONbits.INTF = 0;
        return;
    }

    if (count <= 8) {
        data >>= 1;
        if (bit) { data |= 0x80; state ^= 0x10; } // Set MSB, toggle parity
    } else if (count == 9) {
        if ((((state >> 4)) & 1) != bit) {
            // Parity error
            state = 0;
            INTCONbits.INTF = 0;
            return;
        }
    } else {
        // count == 10
        DEBUG_LED = 1;
        if (bit) decodeScancode(data);
        state = 0;
        INTCONbits.INTF = 0;
        return;
    }

    state = (state & 0xF0) | ((count + 1) & 0x0F);
    INTCONbits.INTF = 0;
}

void setup(void) {
    TRISA = 0xFF;
    TRISB = 0xFF;      // all ports are input
    SR_CLK_DIR = 0;    // output
    SR_DATA_DIR = 0;   // output
    DEBUG_LED_DIR = 0; // output

    SR_CLK = 0;
    SR_DATA = 0;
    DEBUG_LED = 0;

    INTCONbits.INTF = 0;       // clear INT flag
    OPTION_REGbits.INTEDG = 0; // interrupt on falling edge
    INTCONbits.INTE = 1;       // enable INT interrupt
    INTCONbits.GIE = 1;        // enable global interrupts
}

void shiftOutByte(uint8_t data) {
    for (uint8_t i = 0; i < 8; i++) {
        SR_DATA = (data >> (7 - i)) & 1;
        SR_CLK = 1;
        __delay_us(5);
        SR_CLK = 0;
        __delay_us(5);
    }
}

int main() {
    setup();
    while (1) {
        uint8_t head = (bufferIdx >> 4) & BUFFER_MASK;
        uint8_t tail = bufferIdx & BUFFER_MASK;
        if (head != tail) {
            // Check if MCU is ready to receive (INTB high)
            if (INTB) {
                uint8_t data = keyBuffer[tail];
                tail = (tail + 1) & BUFFER_MASK;
                bufferIdx = (uint8_t)((head << 4) | tail);
                shiftOutByte(data);
            }
        } else {
            __delay_us(10); // So you can see the LED blink
            DEBUG_LED = 0;
            __asm("sleep");
        }
    }
    return 0;
}
