#ifndef FLASH_OPS_H
#define FLASH_OPS_H

#include <stdint.h>
#include <stddef.h>

void flash_write_safe(uint32_t offset, const uint8_t *data);
void flash_read_safe(uint32_t offset, uint8_t *buffer);
void flash_erase_safe(uint32_t offset);

#endif // FLASH_OPS_H