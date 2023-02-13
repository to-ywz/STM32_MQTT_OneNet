#ifndef __BOARD__H__
#define __BOARD__H__

#include <stdint.h>

#include "bsp_gpio.h"
#include "delay.h"
#include "led.h"
#include "hw_dht11.h"
#include "ringbuffer.h"
#include "bsp_uart.h"

#define LED0_PIN 105

void board_init(void);

#endif