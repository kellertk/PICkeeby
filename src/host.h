/**
 * PICkeeby - host.h
 * Host bus interface
 */

#ifndef HOST_H
#define HOST_H

#include <stdint.h>
#include <stdbool.h>

#define HOST_QUEUE_SIZE  32

void host_init(void);
void host_send_data(uint8_t data, bool aux);
bool host_data_pending(void);
void host_pump_output(void);
bool host_ibf_active(void);
bool host_get_a0(void);
uint8_t host_read_input(void);

#endif // HOST_H
