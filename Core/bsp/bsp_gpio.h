#ifndef __BSP_GPIO_H__
#define __BSP_GPIO_H__

typedef enum
{
    PIN_MODE_OUTPUT = 0,
    PIN_MODE_INPUT,
    PIN_MODE_INPUT_PULLUP,
    PIN_MODE_INPUT_PULLDOWN,
    PIN_MODE_OUTPUT_OD
} PIN_MODE_t;

void bsp_pin_mode(uint16_t pin, PIN_MODE_t mode);
void bsp_pin_write(uint16_t pin, uint8_t value);
uint8_t bsp_pin_read(uint16_t pin);
void bsp_pin_toggle(uint16_t pin);

#endif
