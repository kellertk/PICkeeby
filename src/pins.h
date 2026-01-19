/**
 * PICkeeby - pins.h
 * Pin definitions and macros
 */

#ifndef PINS_H
#define PINS_H

#include <xc.h>

#define _XTAL_FREQ 32000000UL

#define INPUT   1
#define OUTPUT  0
#define HIGH    1
#define LOW     0
#define ANALOG      1
#define DIGITAL     0
#define PULL_UP_ENABLED      1
#define PULL_UP_DISABLED     0

// RA0 - CLK_OUT
#define CLK_OUT_TRIS                 TRISAbits.TRISA0
#define CLK_OUT_LAT                  LATAbits.LATA0
#define CLK_OUT_PORT                 PORTAbits.RA0
#define CLK_OUT_WPU                  WPUAbits.WPUA0
#define CLK_OUT_OD                   ODCONAbits.ODCA0
#define CLK_OUT_ANS                  ANSELAbits.ANSA0
#define CLK_OUT_SetHigh()            do { LATAbits.LATA0 = 1; } while(0)
#define CLK_OUT_SetLow()             do { LATAbits.LATA0 = 0; } while(0)
#define CLK_OUT_Toggle()             do { LATAbits.LATA0 = ~LATAbits.LATA0; } while(0)
#define CLK_OUT_GetValue()           PORTAbits.RA0
#define CLK_OUT_SetDigitalInput()    do { TRISAbits.TRISA0 = 1; } while(0)
#define CLK_OUT_SetDigitalOutput()   do { TRISAbits.TRISA0 = 0; } while(0)
#define CLK_OUT_SetPullup()          do { WPUAbits.WPUA0 = 1; } while(0)
#define CLK_OUT_ResetPullup()        do { WPUAbits.WPUA0 = 0; } while(0)
#define CLK_OUT_SetPushPull()        do { ODCONAbits.ODCA0 = 0; } while(0)
#define CLK_OUT_SetOpenDrain()       do { ODCONAbits.ODCA0 = 1; } while(0)
#define CLK_OUT_SetAnalogMode()      do { ANSELAbits.ANSA0 = 1; } while(0)
#define CLK_OUT_SetDigitalMode()     do { ANSELAbits.ANSA0 = 0; } while(0)

// RA1 - OE_INB
#define OE_INB_TRIS                 TRISAbits.TRISA1
#define OE_INB_LAT                  LATAbits.LATA1
#define OE_INB_PORT                 PORTAbits.RA1
#define OE_INB_WPU                  WPUAbits.WPUA1
#define OE_INB_OD                   ODCONAbits.ODCA1
#define OE_INB_ANS                  ANSELAbits.ANSA1
#define OE_INB_SetHigh()            do { LATAbits.LATA1 = 1; } while(0)
#define OE_INB_SetLow()             do { LATAbits.LATA1 = 0; } while(0)
#define OE_INB_Toggle()             do { LATAbits.LATA1 = ~LATAbits.LATA1; } while(0)
#define OE_INB_GetValue()           PORTAbits.RA1
#define OE_INB_SetDigitalInput()    do { TRISAbits.TRISA1 = 1; } while(0)
#define OE_INB_SetDigitalOutput()   do { TRISAbits.TRISA1 = 0; } while(0)
#define OE_INB_SetPullup()          do { WPUAbits.WPUA1 = 1; } while(0)
#define OE_INB_ResetPullup()        do { WPUAbits.WPUA1 = 0; } while(0)
#define OE_INB_SetPushPull()        do { ODCONAbits.ODCA1 = 0; } while(0)
#define OE_INB_SetOpenDrain()       do { ODCONAbits.ODCA1 = 1; } while(0)
#define OE_INB_SetAnalogMode()      do { ANSELAbits.ANSA1 = 1; } while(0)
#define OE_INB_SetDigitalMode()     do { ANSELAbits.ANSA1 = 0; } while(0)

// RA2 - IBF_CLRB
#define IBF_CLRB_TRIS                 TRISAbits.TRISA2
#define IBF_CLRB_LAT                  LATAbits.LATA2
#define IBF_CLRB_PORT                 PORTAbits.RA2
#define IBF_CLRB_WPU                  WPUAbits.WPUA2
#define IBF_CLRB_OD                   ODCONAbits.ODCA2
#define IBF_CLRB_ANS                  ANSELAbits.ANSA2
#define IBF_CLRB_SetHigh()            do { LATAbits.LATA2 = 1; } while(0)
#define IBF_CLRB_SetLow()             do { LATAbits.LATA2 = 0; } while(0)
#define IBF_CLRB_Toggle()             do { LATAbits.LATA2 = ~LATAbits.LATA2; } while(0)
#define IBF_CLRB_GetValue()           PORTAbits.RA2
#define IBF_CLRB_SetDigitalInput()    do { TRISAbits.TRISA2 = 1; } while(0)
#define IBF_CLRB_SetDigitalOutput()   do { TRISAbits.TRISA2 = 0; } while(0)
#define IBF_CLRB_SetPullup()          do { WPUAbits.WPUA2 = 1; } while(0)
#define IBF_CLRB_ResetPullup()        do { WPUAbits.WPUA2 = 0; } while(0)
#define IBF_CLRB_SetPushPull()        do { ODCONAbits.ODCA2 = 0; } while(0)
#define IBF_CLRB_SetOpenDrain()       do { ODCONAbits.ODCA2 = 1; } while(0)
#define IBF_CLRB_SetAnalogMode()      do { ANSELAbits.ANSA2 = 1; } while(0)
#define IBF_CLRB_SetDigitalMode()     do { ANSELAbits.ANSA2 = 0; } while(0)

// RA4 - IBF (Input Buffer Full from GAL)
#define IBF_TRIS                 TRISAbits.TRISA4
#define IBF_LAT                  LATAbits.LATA4
#define IBF_PORT                 PORTAbits.RA4
#define IBF_WPU                  WPUAbits.WPUA4
#define IBF_OD                   ODCONAbits.ODCA4
#define IBF_ANS                  ANSELAbits.ANSA4
#define IBF_GetValue()           PORTAbits.RA4
#define IBF_SetDigitalInput()    do { TRISAbits.TRISA4 = 1; } while(0)
#define IBF_SetDigitalOutput()   do { TRISAbits.TRISA4 = 0; } while(0)
#define IBF_SetPullup()          do { WPUAbits.WPUA4 = 1; } while(0)
#define IBF_ResetPullup()        do { WPUAbits.WPUA4 = 0; } while(0)
#define IBF_SetAnalogMode()      do { ANSELAbits.ANSA4 = 1; } while(0)
#define IBF_SetDigitalMode()     do { ANSELAbits.ANSA4 = 0; } while(0)

// RA5 - AUXB
#define AUXB_TRIS                 TRISAbits.TRISA5
#define AUXB_LAT                  LATAbits.LATA5
#define AUXB_PORT                 PORTAbits.RA5
#define AUXB_WPU                  WPUAbits.WPUA5
#define AUXB_OD                   ODCONAbits.ODCA5
#define AUXB_ANS                  ANSELAbits.ANSA5
#define AUXB_SetHigh()            do { LATAbits.LATA5 = 1; } while(0)
#define AUXB_SetLow()             do { LATAbits.LATA5 = 0; } while(0)
#define AUXB_Toggle()             do { LATAbits.LATA5 = ~LATAbits.LATA5; } while(0)
#define AUXB_GetValue()           PORTAbits.RA5
#define AUXB_SetDigitalInput()    do { TRISAbits.TRISA5 = 1; } while(0)
#define AUXB_SetDigitalOutput()   do { TRISAbits.TRISA5 = 0; } while(0)
#define AUXB_SetPullup()          do { WPUAbits.WPUA5 = 1; } while(0)
#define AUXB_ResetPullup()        do { WPUAbits.WPUA5 = 0; } while(0)
#define AUXB_SetPushPull()        do { ODCONAbits.ODCA5 = 0; } while(0)
#define AUXB_SetOpenDrain()       do { ODCONAbits.ODCA5 = 1; } while(0)
#define AUXB_SetAnalogMode()      do { ANSELAbits.ANSA5 = 1; } while(0)
#define AUXB_SetDigitalMode()     do { ANSELAbits.ANSA5 = 0; } while(0)

// RB4 - PS2_CLK1
#define PS2_CLK1_TRIS                 TRISBbits.TRISB4
#define PS2_CLK1_LAT                  LATBbits.LATB4
#define PS2_CLK1_PORT                 PORTBbits.RB4
#define PS2_CLK1_WPU                  WPUBbits.WPUB4
#define PS2_CLK1_OD                   ODCONBbits.ODCB4
#define PS2_CLK1_ANS                  ANSELBbits.ANSB4
#define PS2_CLK1_SetHigh()            do { LATBbits.LATB4 = 1; } while(0)
#define PS2_CLK1_SetLow()             do { LATBbits.LATB4 = 0; } while(0)
#define PS2_CLK1_Toggle()             do { LATBbits.LATB4 = ~LATBbits.LATB4; } while(0)
#define PS2_CLK1_GetValue()           PORTBbits.RB4
#define PS2_CLK1_SetDigitalInput()    do { TRISBbits.TRISB4 = 1; } while(0)
#define PS2_CLK1_SetDigitalOutput()   do { TRISBbits.TRISB4 = 0; } while(0)
#define PS2_CLK1_SetPullup()          do { WPUBbits.WPUB4 = 1; } while(0)
#define PS2_CLK1_ResetPullup()        do { WPUBbits.WPUB4 = 0; } while(0)
#define PS2_CLK1_SetPushPull()        do { ODCONBbits.ODCB4 = 0; } while(0)
#define PS2_CLK1_SetOpenDrain()       do { ODCONBbits.ODCB4 = 1; } while(0)
#define PS2_CLK1_SetAnalogMode()      do { ANSELBbits.ANSB4 = 1; } while(0)
#define PS2_CLK1_SetDigitalMode()     do { ANSELBbits.ANSB4 = 0; } while(0)

// RB5 - PS2_DATA1
#define PS2_DATA1_TRIS                 TRISBbits.TRISB5
#define PS2_DATA1_LAT                  LATBbits.LATB5
#define PS2_DATA1_PORT                 PORTBbits.RB5
#define PS2_DATA1_WPU                  WPUBbits.WPUB5
#define PS2_DATA1_OD                   ODCONBbits.ODCB5
#define PS2_DATA1_ANS                  ANSELBbits.ANSB5
#define PS2_DATA1_SetHigh()            do { LATBbits.LATB5 = 1; } while(0)
#define PS2_DATA1_SetLow()             do { LATBbits.LATB5 = 0; } while(0)
#define PS2_DATA1_Toggle()             do { LATBbits.LATB5 = ~LATBbits.LATB5; } while(0)
#define PS2_DATA1_GetValue()           PORTBbits.RB5
#define PS2_DATA1_SetDigitalInput()    do { TRISBbits.TRISB5 = 1; } while(0)
#define PS2_DATA1_SetDigitalOutput()   do { TRISBbits.TRISB5 = 0; } while(0)
#define PS2_DATA1_SetPullup()          do { WPUBbits.WPUB5 = 1; } while(0)
#define PS2_DATA1_ResetPullup()        do { WPUBbits.WPUB5 = 0; } while(0)
#define PS2_DATA1_SetPushPull()        do { ODCONBbits.ODCB5 = 0; } while(0)
#define PS2_DATA1_SetOpenDrain()       do { ODCONBbits.ODCB5 = 1; } while(0)
#define PS2_DATA1_SetAnalogMode()      do { ANSELBbits.ANSB5 = 1; } while(0)
#define PS2_DATA1_SetDigitalMode()     do { ANSELBbits.ANSB5 = 0; } while(0)

// RB6 - PS2_CLK2
#define PS2_CLK2_TRIS                 TRISBbits.TRISB6
#define PS2_CLK2_LAT                  LATBbits.LATB6
#define PS2_CLK2_PORT                 PORTBbits.RB6
#define PS2_CLK2_WPU                  WPUBbits.WPUB6
#define PS2_CLK2_OD                   ODCONBbits.ODCB6
#define PS2_CLK2_ANS                  ANSELBbits.ANSB6
#define PS2_CLK2_SetHigh()            do { LATBbits.LATB6 = 1; } while(0)
#define PS2_CLK2_SetLow()             do { LATBbits.LATB6 = 0; } while(0)
#define PS2_CLK2_Toggle()             do { LATBbits.LATB6 = ~LATBbits.LATB6; } while(0)
#define PS2_CLK2_GetValue()           PORTBbits.RB6
#define PS2_CLK2_SetDigitalInput()    do { TRISBbits.TRISB6 = 1; } while(0)
#define PS2_CLK2_SetDigitalOutput()   do { TRISBbits.TRISB6 = 0; } while(0)
#define PS2_CLK2_SetPullup()          do { WPUBbits.WPUB6 = 1; } while(0)
#define PS2_CLK2_ResetPullup()        do { WPUBbits.WPUB6 = 0; } while(0)
#define PS2_CLK2_SetPushPull()        do { ODCONBbits.ODCB6 = 0; } while(0)
#define PS2_CLK2_SetOpenDrain()       do { ODCONBbits.ODCB6 = 1; } while(0)
#define PS2_CLK2_SetAnalogMode()      do { ANSELBbits.ANSB6 = 1; } while(0)
#define PS2_CLK2_SetDigitalMode()     do { ANSELBbits.ANSB6 = 0; } while(0)

// RB7 - PS2_DATA2
#define PS2_DATA2_TRIS                 TRISBbits.TRISB7
#define PS2_DATA2_LAT                  LATBbits.LATB7
#define PS2_DATA2_PORT                 PORTBbits.RB7
#define PS2_DATA2_WPU                  WPUBbits.WPUB7
#define PS2_DATA2_OD                   ODCONBbits.ODCB7
#define PS2_DATA2_ANS                  ANSELBbits.ANSB7
#define PS2_DATA2_SetHigh()            do { LATBbits.LATB7 = 1; } while(0)
#define PS2_DATA2_SetLow()             do { LATBbits.LATB7 = 0; } while(0)
#define PS2_DATA2_Toggle()             do { LATBbits.LATB7 = ~LATBbits.LATB7; } while(0)
#define PS2_DATA2_GetValue()           PORTBbits.RB7
#define PS2_DATA2_SetDigitalInput()    do { TRISBbits.TRISB7 = 1; } while(0)
#define PS2_DATA2_SetDigitalOutput()   do { TRISBbits.TRISB7 = 0; } while(0)
#define PS2_DATA2_SetPullup()          do { WPUBbits.WPUB7 = 1; } while(0)
#define PS2_DATA2_ResetPullup()        do { WPUBbits.WPUB7 = 0; } while(0)
#define PS2_DATA2_SetPushPull()        do { ODCONBbits.ODCB7 = 0; } while(0)
#define PS2_DATA2_SetOpenDrain()       do { ODCONBbits.ODCB7 = 1; } while(0)
#define PS2_DATA2_SetAnalogMode()      do { ANSELBbits.ANSB7 = 1; } while(0)
#define PS2_DATA2_SetDigitalMode()     do { ANSELBbits.ANSB7 = 0; } while(0)

// RC0-RC7 - IDATA0-IDATA7 (8-bit data bus)
#define IDATA0_TRIS                 TRISCbits.TRISC0
#define IDATA0_LAT                  LATCbits.LATC0
#define IDATA0_PORT                 PORTCbits.RC0
#define IDATA0_SetHigh()            do { LATCbits.LATC0 = 1; } while(0)
#define IDATA0_SetLow()             do { LATCbits.LATC0 = 0; } while(0)
#define IDATA0_GetValue()           PORTCbits.RC0
#define IDATA0_SetDigitalInput()    do { TRISCbits.TRISC0 = 1; } while(0)
#define IDATA0_SetDigitalOutput()   do { TRISCbits.TRISC0 = 0; } while(0)

#define IDATA1_TRIS                 TRISCbits.TRISC1
#define IDATA1_LAT                  LATCbits.LATC1
#define IDATA1_PORT                 PORTCbits.RC1
#define IDATA1_SetHigh()            do { LATCbits.LATC1 = 1; } while(0)
#define IDATA1_SetLow()             do { LATCbits.LATC1 = 0; } while(0)
#define IDATA1_GetValue()           PORTCbits.RC1
#define IDATA1_SetDigitalInput()    do { TRISCbits.TRISC1 = 1; } while(0)
#define IDATA1_SetDigitalOutput()   do { TRISCbits.TRISC1 = 0; } while(0)

#define IDATA2_TRIS                 TRISCbits.TRISC2
#define IDATA2_LAT                  LATCbits.LATC2
#define IDATA2_PORT                 PORTCbits.RC2
#define IDATA2_SetHigh()            do { LATCbits.LATC2 = 1; } while(0)
#define IDATA2_SetLow()             do { LATCbits.LATC2 = 0; } while(0)
#define IDATA2_GetValue()           PORTCbits.RC2
#define IDATA2_SetDigitalInput()    do { TRISCbits.TRISC2 = 1; } while(0)
#define IDATA2_SetDigitalOutput()   do { TRISCbits.TRISC2 = 0; } while(0)

#define IDATA3_TRIS                 TRISCbits.TRISC3
#define IDATA3_LAT                  LATCbits.LATC3
#define IDATA3_PORT                 PORTCbits.RC3
#define IDATA3_SetHigh()            do { LATCbits.LATC3 = 1; } while(0)
#define IDATA3_SetLow()             do { LATCbits.LATC3 = 0; } while(0)
#define IDATA3_GetValue()           PORTCbits.RC3
#define IDATA3_SetDigitalInput()    do { TRISCbits.TRISC3 = 1; } while(0)
#define IDATA3_SetDigitalOutput()   do { TRISCbits.TRISC3 = 0; } while(0)

#define IDATA4_TRIS                 TRISCbits.TRISC4
#define IDATA4_LAT                  LATCbits.LATC4
#define IDATA4_PORT                 PORTCbits.RC4
#define IDATA4_SetHigh()            do { LATCbits.LATC4 = 1; } while(0)
#define IDATA4_SetLow()             do { LATCbits.LATC4 = 0; } while(0)
#define IDATA4_GetValue()           PORTCbits.RC4
#define IDATA4_SetDigitalInput()    do { TRISCbits.TRISC4 = 1; } while(0)
#define IDATA4_SetDigitalOutput()   do { TRISCbits.TRISC4 = 0; } while(0)

#define IDATA5_TRIS                 TRISCbits.TRISC5
#define IDATA5_LAT                  LATCbits.LATC5
#define IDATA5_PORT                 PORTCbits.RC5
#define IDATA5_SetHigh()            do { LATCbits.LATC5 = 1; } while(0)
#define IDATA5_SetLow()             do { LATCbits.LATC5 = 0; } while(0)
#define IDATA5_GetValue()           PORTCbits.RC5
#define IDATA5_SetDigitalInput()    do { TRISCbits.TRISC5 = 1; } while(0)
#define IDATA5_SetDigitalOutput()   do { TRISCbits.TRISC5 = 0; } while(0)

#define IDATA6_TRIS                 TRISCbits.TRISC6
#define IDATA6_LAT                  LATCbits.LATC6
#define IDATA6_PORT                 PORTCbits.RC6
#define IDATA6_SetHigh()            do { LATCbits.LATC6 = 1; } while(0)
#define IDATA6_SetLow()             do { LATCbits.LATC6 = 0; } while(0)
#define IDATA6_GetValue()           PORTCbits.RC6
#define IDATA6_SetDigitalInput()    do { TRISCbits.TRISC6 = 1; } while(0)
#define IDATA6_SetDigitalOutput()   do { TRISCbits.TRISC6 = 0; } while(0)

#define IDATA7_TRIS                 TRISCbits.TRISC7
#define IDATA7_LAT                  LATCbits.LATC7
#define IDATA7_PORT                 PORTCbits.RC7
#define IDATA7_SetHigh()            do { LATCbits.LATC7 = 1; } while(0)
#define IDATA7_SetLow()             do { LATCbits.LATC7 = 0; } while(0)
#define IDATA7_GetValue()           PORTCbits.RC7
#define IDATA7_SetDigitalInput()    do { TRISCbits.TRISC7 = 1; } while(0)
#define IDATA7_SetDigitalOutput()   do { TRISCbits.TRISC7 = 0; } while(0)

// Convenience macros for PORTC data bus
#define IDATA_PORT                  PORTC
#define IDATA_LAT                   LATC
#define IDATA_TRIS                  TRISC

#endif // PINS_H
