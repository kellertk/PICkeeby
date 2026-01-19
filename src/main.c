/**
 * PICkeeby - main.c
 * PS/2 Keyboard Controller for Homebrew Computers
 * i8042-compatible interface
 */

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include "pins.h"
#include "ps2.h"
#include "host.h"
#include "i8042.h"

// CONFIG1
#pragma config FEXTOSC = OFF    // External oscillator disabled
#pragma config RSTOSC = HFINT32 // Power up osc: HFINTOSC with OSCFRQ = 32 MHz
#pragma config CLKOUTEN = OFF   // CLKOUT disabled
#pragma config CSWEN = OFF      // Clock switch disabled
#pragma config FCMEN = ON       // Fail-safe clock monitor enabled

// CONFIG2
#pragma config MCLRE = OFF      // MCLR disabled, RA3 is digital input
#pragma config PWRTE = OFF      // Power-up timer disabled
#pragma config WDTE = OFF       // Watchdog disabled
#pragma config LPBOREN = OFF    // Low-power BOR disabled
#pragma config BOREN = ON       // Brown-out reset enabled
#pragma config BORV = HIGH      // Brown-out voltage trip point high

// CONFIG3
#pragma config PPS1WAY = ON     // PPS locked after one unlock
#pragma config STVREN = ON      // Stack overflow resets device

// CONFIG4
#pragma config WRT = OFF        // Flash write protection off
#pragma config LVP = ON         // Low-voltage programming enabled
#pragma config CP = OFF         // Code protection off
#pragma config CPD = OFF        // Data EEPROM protection off

static void clock_init(void) {
    OSCCON1 = (0 << _OSCCON1_NDIV_POSN) | (6 << _OSCCON1_NOSC_POSN);
    OSCFRQ = (6 << _OSCFRQ_HFFRQ_POSN);  // 32 MHz
}

static void pins_init(void) {
    // Set analog pins to digital
    ANSELA = 0x00;
    ANSELB = 0x00;
    ANSELC = 0x00;

    // RA0: CLK_OUT (output, push-pull)
    // RA1: OE_INB (output, push-pull)
    // RA2: IBF_CLRB (output, push-pull)
    // RA3: A0 (input, directly to CLC1)
    // RA4: IBF (input from GAL, CLC1 clock)
    // RA5: AUXB (output, push-pull)
    TRISA = 0x18;  // RA3, RA4 as inputs
    LATA = 0x04;   // IBF_CLRB high
    ODCONA = 0x00;

    // RB4: PS2_CLK1 (bidirectional, open-drain)
    // RB5: PS2_DATA1 (bidirectional, open-drain)
    // RB6: PS2_CLK2 (bidirectional, open-drain)
    // RB7: PS2_DATA2 (bidirectional, open-drain)
    TRISB = 0xF0;
    LATB = 0x00;
    ODCONB = 0xF0;

    // RC0-7: IDATA bus (output to latches)
    TRISC = 0x00;
    LATC = 0x00;

    // IOC for PS/2 clock falling edges
    IOCBN = 0x50;  // Negative edge on RB4, RB6
    IOCBF = 0x00;

    // Enable IOC interrupt
    PIE0bits.IOCIE = 1;
}

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

int main(void) {
    clock_init();
    pins_init();
    ps2_init();
    host_init();
    i8042_init();

    // Enable interrupts
    INTCONbits.PEIE = 1;
    INTCONbits.GIE = 1;

    while (1) {
        ps2_task();
        i8042_task();

        // Pump output queue to host
        if (host_data_pending()) {
            host_pump_output();
        }

        // Check for host input
        if (host_ibf_active()) {
            bool is_cmd = host_get_a0();
            uint8_t byte = host_read_input();
            i8042_process_byte(byte, is_cmd);
        }
    }
}

