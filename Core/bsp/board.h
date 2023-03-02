#ifndef __BOARD__H__
#define __BOARD__H__

#include <stdint.h>

#include "bsp_gpio.h"
#include "sf_timer.h"
#include "led.h"
#include "ringbuffer.h"
#include "bsp_uart.h"
#include "systick.h"


void board_init(void);
void board_system_reset(void);
uint8_t get_pinnum(char *str);

#endif