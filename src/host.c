/**
 * PICkeeby - host.c
 * Host bus interface via 74HCT574 (output) and 74HCT373 (input) latches
 */

#include <xc.h>
#include "host.h"
#include "pins.h"

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
    IDATA_LAT = 0x00;

    // Control signals
    CLK_OUT_SetLow();
    OE_INB_SetHigh();   // Disable input latch outputs
    IBF_CLRB_SetHigh();
    AUXB_SetLow();

    // Configure CLC1 as D flip-flop to latch A0 on IBF rising edge
    // D input: RA3 (A0), CLK input: RA4 (IBF)
    CLC1SEL0 = 0x03;    // CLCIN0 = RA3 (D input)
    CLC1SEL1 = 0x04;    // CLCIN1 = RA4 (CLK input)
    CLC1SEL2 = 0x00;    // Unused
    CLC1SEL3 = 0x00;    // Unused
    CLC1GLS0 = 0x02;    // Gate 1: CLCIN0 (D)
    CLC1GLS1 = 0x00;    // Gate 2: not used for D-FF
    CLC1GLS2 = 0x08;    // Gate 3: CLCIN1 (CLK)
    CLC1GLS3 = 0x00;    // Gate 4: not used
    CLC1POL = 0x00;     // No inversions
    CLC1CON = 0x84;     // D flip-flop with R, enabled
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
    IDATA_LAT = data;

    // Pulse CLK_OUT to latch data and set OBF
    CLK_OUT_SetHigh();
    __delay_us(1);
    CLK_OUT_SetLow();
}

bool host_ibf_active(void) {
    return IBF_GetValue();
}

bool host_get_a0(void) {
    return CLC1CONbits.LC1OUT;
}

uint8_t host_read_input(void) {
    // Switch internal bus to input
    IDATA_TRIS = 0xFF;

    // Enable input latch outputs
    OE_INB_SetLow();
    __delay_us(1);

    // Read the data
    uint8_t data = IDATA_PORT;

    // Disable input latch outputs
    OE_INB_SetHigh();

    // Switch internal bus back to output
    IDATA_TRIS = 0x00;

    // Clear IBF
    IBF_CLRB_SetLow();
    __delay_us(1);
    IBF_CLRB_SetHigh();

    return data;
}
