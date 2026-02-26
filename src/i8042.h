/**
 * PICkeeby - i8042.h
 * i8042 keyboard controller emulation
 */

#ifndef I8042_H
#define I8042_H

#include <stdint.h>
#include <stdbool.h>

void i8042_init(void);
void i8042_task(void);
void i8042_process_byte(uint8_t byte, bool is_command);

#endif // I8042_H
