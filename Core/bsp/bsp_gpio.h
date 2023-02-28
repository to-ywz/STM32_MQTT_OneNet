#ifndef __BSP_GPIO_H__
#define __BSP_GPIO_H__

#include <stdint.h>

typedef enum
{
    PIN_MODE_OUTPUT = 0,
    PIN_MODE_INPUT,
    PIN_MODE_INPUT_PULLUP,
    PIN_MODE_INPUT_PULLDOWN,
    PIN_MODE_OUTPUT_OD
} PIN_MODE_ET;

typedef enum
{
    PIN_IRQ_MODE_RISING = 0,
    PIN_IRQ_MODE_FALLING,
    PIN_IRQ_MODE_RISING_FALLING
} PIN_IQR_MODE_ET;

void bsp_pin_mode(uint16_t pin, PIN_MODE_ET mode);
void bsp_pin_write(uint16_t pin, uint8_t value);
uint8_t bsp_pin_read(uint16_t pin);
void bsp_pin_toggle(uint16_t pin);
void bsp_pin_exit_init(uint16_t pin_num, PIN_IQR_MODE_ET mode, uint8_t priority);

#endif
