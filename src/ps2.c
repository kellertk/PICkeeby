/**
 * PICkeeby - ps2.c
 * PS/2 protocol implementation (bit-banged RX/TX)
 */

#include <xc.h>
#include "ps2.h"
#include "pins.h"

ps2_port_t ps2_ports[2];

static inline void queue_init(ps2_queue_t *q) {
    q->head = 0;
    q->tail = 0;
}

static inline bool queue_empty(ps2_queue_t *q) {
    return q->head == q->tail;
}

static inline bool queue_full(ps2_queue_t *q) {
    return ((q->head + 1) % PS2_QUEUE_SIZE) == q->tail;
}

static inline void queue_push(ps2_queue_t *q, uint8_t data) {
    if (!queue_full(q)) {
        q->data[q->head] = data;
        q->head = (q->head + 1) % PS2_QUEUE_SIZE;
    }
}

static inline uint8_t queue_pop(ps2_queue_t *q) {
    if (queue_empty(q)) return 0;
    uint8_t data = q->data[q->tail];
    q->tail = (q->tail + 1) % PS2_QUEUE_SIZE;
    return data;
}

static inline bool ps2_get_data(uint8_t port) {
    return (port == PS2_PORT1) ? PS2_DATA1_GetValue() : PS2_DATA2_GetValue();
}

static inline void ps2_set_data(uint8_t port, bool high) {
    if (port == PS2_PORT1) {
        if (high) {
            PS2_DATA1_SetDigitalInput();
        } else {
            PS2_DATA1_LAT = 0;
            PS2_DATA1_SetDigitalOutput();
        }
    } else {
        if (high) {
            PS2_DATA2_SetDigitalInput();
        } else {
            PS2_DATA2_LAT = 0;
            PS2_DATA2_SetDigitalOutput();
        }
    }
}

static inline void ps2_set_clock(uint8_t port, bool high) {
    if (port == PS2_PORT1) {
        if (high) {
            PS2_CLK1_SetDigitalInput();
        } else {
            PS2_CLK1_LAT = 0;
            PS2_CLK1_SetDigitalOutput();
        }
    } else {
        if (high) {
            PS2_CLK2_SetDigitalInput();
        } else {
            PS2_CLK2_LAT = 0;
            PS2_CLK2_SetDigitalOutput();
        }
    }
}

void ps2_init(void) {
    for (uint8_t i = 0; i < 2; i++) {
        ps2_ports[i].state = PS2_STATE_IDLE;
        ps2_ports[i].bit_count = 0;
        ps2_ports[i].shift_reg = 0;
        ps2_ports[i].parity = 0;
        ps2_ports[i].tx_byte = 0;
        ps2_ports[i].timeout = 0;
        ps2_ports[i].inhibit = false;
        queue_init(&ps2_ports[i].rx_queue);
        queue_init(&ps2_ports[i].tx_queue);
    }

    PS2_CLK1_SetDigitalInput();
    PS2_DATA1_SetDigitalInput();
    PS2_CLK2_SetDigitalInput();
    PS2_DATA2_SetDigitalInput();

    PS2_CLK1_SetOpenDrain();
    PS2_DATA1_SetOpenDrain();
    PS2_CLK2_SetOpenDrain();
    PS2_DATA2_SetOpenDrain();
}

void ps2_clock_isr(uint8_t port) {
    ps2_port_t *p = &ps2_ports[port];

    if (p->inhibit) return;

    p->timeout = 0;

    switch (p->state) {
    case PS2_STATE_IDLE:
        if (!ps2_get_data(port)) {
            p->state = PS2_STATE_RX;
            p->bit_count = 0;
            p->shift_reg = 0;
            p->parity = 0;
        }
        break;

    case PS2_STATE_RX:
        if (p->bit_count < 8) {
            bool bit = ps2_get_data(port);
            p->shift_reg >>= 1;
            if (bit) {
                p->shift_reg |= 0x80;
                p->parity ^= 1;
            }
            p->bit_count++;
        } else if (p->bit_count == 8) {
            if (ps2_get_data(port)) p->parity ^= 1;
            p->bit_count++;
        } else {
            if (ps2_get_data(port) && p->parity) {
                queue_push(&p->rx_queue, p->shift_reg);
            }
            p->state = PS2_STATE_IDLE;
        }
        break;

    case PS2_STATE_TX:
        if (p->bit_count < 8) {
            bool bit = p->tx_byte & 1;
            ps2_set_data(port, bit);
            p->tx_byte >>= 1;
            if (bit) p->parity ^= 1;
            p->bit_count++;
        } else if (p->bit_count == 8) {
            ps2_set_data(port, !p->parity);
            p->bit_count++;
        } else if (p->bit_count == 9) {
            ps2_set_data(port, true);
            p->bit_count++;
        } else {
            p->state = PS2_STATE_TX_ACK;
        }
        break;

    case PS2_STATE_TX_ACK:
        ps2_set_data(port, true);
        p->state = PS2_STATE_IDLE;
        break;
    }
}

void ps2_task(void) {
    for (uint8_t port = 0; port < 2; port++) {
        ps2_port_t *p = &ps2_ports[port];

        // Timeout recovery: if stuck mid-byte, reset to idle
        if (p->state != PS2_STATE_IDLE) {
            if (++p->timeout > 200) {
                ps2_set_data(port, true);
                ps2_set_clock(port, true);
                p->state = PS2_STATE_IDLE;
                p->timeout = 0;
            }
        }

        if (p->state == PS2_STATE_IDLE && !queue_empty(&p->tx_queue)) {
            p->tx_byte = queue_pop(&p->tx_queue);
            p->parity = 0;
            p->bit_count = 0;

            ps2_set_clock(port, false);
            __delay_us(150);
            ps2_set_data(port, false);
            __delay_us(10);
            ps2_set_clock(port, true);

            p->state = PS2_STATE_TX;
        }
    }
}

bool ps2_rx_available(uint8_t port) {
    return !queue_empty(&ps2_ports[port].rx_queue);
}

uint8_t ps2_rx_read(uint8_t port) {
    return queue_pop(&ps2_ports[port].rx_queue);
}

void ps2_tx_write(uint8_t port, uint8_t data) {
    queue_push(&ps2_ports[port].tx_queue, data);
}

void ps2_inhibit(uint8_t port, bool inhibit) {
    ps2_ports[port].inhibit = inhibit;
    if (inhibit) {
        ps2_set_clock(port, false);
    } else {
        ps2_set_clock(port, true);
    }
}
