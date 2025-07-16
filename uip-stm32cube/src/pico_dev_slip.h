#ifndef SLIP_DEV_H
#define SLIP_DEV_H

#include <pico_stack.h>
#include <pico_device.h>
#include <pico_queue.h>
#include <pico_config.h>
#include <stdint.h>
#include <string.h>

/* Hypothetical BSP serial functions */
extern int BSP_serial_init(uint32_t base, uint32_t baudrate);
extern int BSP_serial_write(uint32_t base, const uint8_t *data, uint32_t len);
extern int BSP_serial_read(uint32_t base, uint8_t *data, uint32_t len);

struct pico_device *pico_slip_create(const char *name, uint32_t serial_base, uint32_t baudrate);
#endif // SLIP_DEV_H
