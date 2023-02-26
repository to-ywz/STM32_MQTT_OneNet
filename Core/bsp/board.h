#ifndef __BOARD__H__
#define __BOARD__H__

#include <stdint.h>

#include "bsp_gpio.h"
#include "delay.h"
#include "led.h"
#include "ringbuffer.h"
#include "bsp_uart.h"



void board_init(void);

#endif