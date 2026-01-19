/**
 * PICkeeby - ps2.h
 * PS/2 protocol interface
 */

#ifndef PS2_H
#define PS2_H

#include <stdint.h>
#include <stdbool.h>

#define PS2_PORT1  0
#define PS2_PORT2  1

#define PS2_QUEUE_SIZE  16

typedef struct {
    uint8_t data[PS2_QUEUE_SIZE];
    uint8_t head;
    uint8_t tail;
} ps2_queue_t;

typedef enum {
    PS2_STATE_IDLE,
    PS2_STATE_RX,
    PS2_STATE_TX,
    PS2_STATE_TX_ACK
} ps2_state_t;

typedef struct {
    ps2_state_t state;
    uint8_t bit_count;
    uint8_t shift_reg;
    uint8_t parity;
    uint8_t tx_byte;
    uint8_t timeout;
    bool inhibit;
    ps2_queue_t rx_queue;
    ps2_queue_t tx_queue;
} ps2_port_t;

extern ps2_port_t ps2_ports[2];

void ps2_init(void);
void ps2_clock_isr(uint8_t port);
void ps2_task(void);

bool ps2_rx_available(uint8_t port);
uint8_t ps2_rx_read(uint8_t port);
void ps2_tx_write(uint8_t port, uint8_t data);
void ps2_inhibit(uint8_t port, bool inhibit);

#endif // PS2_H
