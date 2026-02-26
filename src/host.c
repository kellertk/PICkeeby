/**
 * PICkeeby - host.c
 * Host bus interface via 74HCT574 (output) and 74HCT373 (input) latches
 */

#include <xc.h>
#include "host.h"
#include "pins.h"

// Remap data byte to PORTC bit layout for writing
static uint8_t idata_to_port(uint8_t val) {
    return (uint8_t)(
           ((val & 0x01) << 5)   // IDATA0 → RC5
         | ((val & 0x02) >> 1)   // IDATA1 → RC0
         | ((val & 0x04) << 2)   // IDATA2 → RC4
         | ((val & 0x08) >> 2)   // IDATA3 → RC1
         | ((val & 0x10) >> 1)   // IDATA4 → RC3
         | ((val & 0x20) >> 3)   // IDATA5 → RC2
         | (val & 0xC0));        // IDATA6,7 unchanged
}

// Remap PORTC bit layout to data byte for reading
static uint8_t idata_from_port(uint8_t val) {
    return (uint8_t)(
           ((val & 0x01) << 1)   // RC0 → IDATA1
         | ((val & 0x02) << 2)   // RC1 → IDATA3
         | ((val & 0x04) << 3)   // RC2 → IDATA5
         | ((val & 0x08) << 1)   // RC3 → IDATA4
         | ((val & 0x10) >> 2)   // RC4 → IDATA2
         | ((val & 0x20) >> 5)   // RC5 → IDATA0
         | (val & 0xC0));        // bits 6,7 unchanged
}

#define IDATA_Write(data)  do { LATC = idata_to_port(data); } while(0)
#define IDATA_Read()       idata_from_port(PORTC)

typedef struct {
    uint8_t data[HOST_QUEUE_SIZE];
    bool aux[HOST_QUEUE_SIZE];
    uint8_t head;
    uint8_t tail;
} host_queue_t;

static host_queue_t output_queue;

static inline bool queue_empty(void) {
    return output_queue.head == output_queue.tail;
}

static inline bool queue_full(void) {
    return ((output_queue.head + 1) % HOST_QUEUE_SIZE) == output_queue.tail;
}

void host_init(void) {
    output_queue.head = 0;
    output_queue.tail = 0;

    // IDATA bus as output (to latches)
    IDATA_TRIS = 0x00;
    IDATA_Write(0x00);

    // Control signals
    CLK_OUT_SetLow();
    OE_INB_SetHigh();   // Disable input latch outputs
    IBF_CLRB_SetHigh();
    AUXB_SetLow();

}

void host_send_data(uint8_t data, bool aux) {
    if (!queue_full()) {
        output_queue.data[output_queue.head] = data;
        output_queue.aux[output_queue.head] = aux;
        output_queue.head = (output_queue.head + 1) % HOST_QUEUE_SIZE;
    }
}

bool host_data_pending(void) {
    return !queue_empty();
}

void host_pump_output(void) {
    if (queue_empty()) return;

    uint8_t data = output_queue.data[output_queue.tail];
    bool aux = output_queue.aux[output_queue.tail];
    output_queue.tail = (output_queue.tail + 1) % HOST_QUEUE_SIZE;

    // Set AUXB signal to indicate source port
    if (aux) {
        AUXB_SetHigh();
    } else {
        AUXB_SetLow();
    }

    // Ensure input latch outputs are disabled
    OE_INB_SetHigh();

    // Drive data onto internal bus
    IDATA_Write(data);

    // Pulse CLK_OUT to latch data and set OBF
    CLK_OUT_SetHigh();
    __delay_us(1);
    CLK_OUT_SetLow();
}

bool host_ibf_active(void) {
    return IBF_GetValue();
}

bool host_get_a0(void) {
    return A0_GetValue();
}

uint8_t host_read_input(void) {
    // Switch internal bus to input
    IDATA_TRIS = 0xFF;

    // Enable input latch outputs
    OE_INB_SetLow();
    __delay_us(1);

    // Read the data
    uint8_t data = IDATA_Read();

    // Disable input latch outputs
    OE_INB_SetHigh();

    // Switch internal bus back to output
    IDATA_TRIS = 0x00;

    // Clear IBF — async reset clears all registered outputs immediately
    IBF_CLRB_SetLow();
    NOP();
    IBF_CLRB_SetHigh();

    return data;
}
