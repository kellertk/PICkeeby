#ifndef PS2_SEND_H
#define PS2_SEND_H

#include <stdint.h>

// Transmission timeout flag (set by Timer0 ISR when INTE disabled)
extern volatile uint8_t tx_timeout;

// Echo monitoring flag (set by Timer1 ISR)
extern volatile uint8_t echo_timeout;

// PS/2 command functions
void ps2_setLEDs(uint8_t leds);            // 0xED: Set LEDs (bit 0=scroll, 1=num, 2=caps)
void ps2_echo(void);                       // 0xEE: Echo (diagnostic)
void ps2_setTypematic(uint8_t rate);       // 0xF3: Set typematic rate/delay
void ps2_enable(void);                     // 0xF4: Enable scanning
void ps2_disable(void);                    // 0xF5: Disable scanning
void ps2_setDefaults(void);                // 0xF6: Set default parameters
void ps2_reset(void);                      // 0xFF: Reset keyboard
void ps2_initKeyboard(void);               // Initialize keyboard: LEDs off, numlock on, typematic 500ms/30Hz

// Process command queue (call from main loop)
void ps2_processCommands(void);

#endif
