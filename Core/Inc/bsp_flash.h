#ifndef _BSP_FLASH_
#define _BSP_FLASH_

#include <stdint.h>

uint32_t flash_write_data(uint32_t address, uint8_t *data, uint32_t size);
uint32_t flash_read_data(uint32_t address, uint8_t *data, uint32_t size);

#endif
// FILE END
