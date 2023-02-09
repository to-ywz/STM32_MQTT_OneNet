/**
 * @file bsp_gpio.c
 * @author BlackSheep (blacksheep.208h@gmail.com)
 * @brief   STM32 GPIO通用初始化
 * @version 0.1
 * @date 2023-02-07
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "stm32f4xx_hal.h"
#include "bsp_gpio.h"
#include "bsp_gpio_def.h"

static void hw_pin_mode(uint16_t pin_num, PIN_MODE_t mode);
static void hw_pin_write(uint16_t pin_num, uint8_t value);
static uint8_t hw_pin_read(uint16_t pin_num);
static void hw_pin_toggle(uint16_t pin_num);

//=================================软件层次==========================================
/**
 * @brief       配置引脚的GPIO工作模式
 *
 * @param pin   要配置的引脚编号
 * @param mode  GPIO的工作模式
 */
void bsp_pin_mode(uint16_t pin_num, PIN_MODE_t mode)
{
    hw_pin_mode(pin_num, mode);
}

/**
 * @brief       写入电平
 *
 * @param pin   要修改电平的引脚编号
 * @param value 要写入的电平
 */
void bsp_pin_write(uint16_t pin_num, uint8_t value)
{
    hw_pin_write(pin_num, value);
}

/**
 * @brief      读取引脚电平
 *
 * @param pin  要读取的引脚编号
 * @retval     引脚电平状态
 */
uint8_t bsp_pin_read(uint16_t pin_num)
{
    return hw_pin_read(pin_num);
}

/**
 * @brief       翻转电平
 *
 * @param pin 要 翻转的引脚的编号
 */
void bsp_pin_toggle(uint16_t pin_num)
{
    hw_pin_toggle(pin_num);
}

//=================================硬件层次==========================================
/* 这里 可以实现多种MCU的初始化,减少代码大小可以使用宏来实现 */
/**
 * @brief       配置引脚的GPIO工作模式
 *
 * @param pin   要配置的引脚的编号
 * @param mode  GPIO的工作模式
 */
static void hw_pin_mode(uint16_t pin_num, PIN_MODE_t mode)
{

    uint16_t pin = 0;
    uint8_t port_num = 0;
    GPIO_TypeDef *port = 0;
    GPIO_InitTypeDef GPIO_InitSture = {};

    // 获取需要配置引脚的信息
    pin = 1U << GET_PIN(pin_num);
    port_num = GET_PORT(pin_num);
    port = (GPIO_TypeDef *)(GPIOA_BASE + port_num * 0x400);

    // * 使能对应时钟
    RCC->AHB1ENR |= 1U << port_num;

    /* 配置初始化结构体模式 */
    GPIO_InitSture.Pin = pin;
    GPIO_InitSture.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitSture.Pull = GPIO_NOPULL;
    GPIO_InitSture.Speed = GPIO_SPEED_FREQ_HIGH;

    if (mode == PIN_MODE_OUTPUT)
    {
        /* 输出模式配置 */
        GPIO_InitSture.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitSture.Pull = GPIO_NOPULL;
    }
    else if (mode == PIN_MODE_INPUT)
    {
        /* 浮空输入模式 */
        GPIO_InitSture.Mode = GPIO_MODE_INPUT;
        GPIO_InitSture.Pull = GPIO_NOPULL;
    }
    else if (mode == PIN_MODE_INPUT_PULLUP)
    {
        /* 上拉输入模式 */
        GPIO_InitSture.Mode = GPIO_MODE_INPUT;
        GPIO_InitSture.Pull = GPIO_PULLUP;
    }
    else if (mode == PIN_MODE_INPUT_PULLDOWN)
    {
        /* 下拉输入模式 */
        GPIO_InitSture.Mode = GPIO_MODE_INPUT;
        GPIO_InitSture.Pull = GPIO_PULLDOWN;
    }
    else if (mode == PIN_MODE_OUTPUT_OD)
    {
        /* 开漏输出模式 */
        GPIO_InitSture.Mode = GPIO_MODE_OUTPUT_OD;
        GPIO_InitSture.Pull = GPIO_NOPULL;
    }

    HAL_GPIO_Init(port, &GPIO_InitSture);
}

/**
 * @brief       写入电平
 *
 * @param pin   要修改电平的引脚的编号
 * @param value 要写入的电平
 */
static void hw_pin_write(uint16_t pin_num, uint8_t value)
{
    uint16_t pin = 0;
    uint8_t port_num = 0;
    GPIO_TypeDef *port = 0;

    pin = 1U << GET_PIN(pin_num);
    port_num = GET_PORT(pin_num);
    port = (GPIO_TypeDef *)(GPIOA_BASE + port_num * 0x400);

    HAL_GPIO_WritePin(port, pin, value);
}

/**
 * @brief      读取引脚电平
 *
 * @param pin  要读取的引脚编号
 * @retval     引脚电平状态
 */
static uint8_t hw_pin_read(uint16_t pin_num)
{
    uint16_t pin = 0;
    uint8_t port_num = 0;
    GPIO_TypeDef *port = 0;

    pin = 1U << GET_PIN(pin_num);
    port_num = GET_PORT(pin_num);
    port = (GPIO_TypeDef *)(GPIOA_BASE + port_num * 0x400);

    return HAL_GPIO_ReadPin(port, pin);
}

/**
 * @brief       翻转电平
 *
 * @param pin 要 翻转的引脚编号
 */
static void hw_pin_toggle(uint16_t pin_num)
{
    uint16_t pin = 0;
    uint8_t port_num = 0;
    GPIO_TypeDef *port = 0;

    pin = 1U << GET_PIN(pin_num);
    port_num = GET_PORT(pin_num);
    port = (GPIO_TypeDef *)(GPIOA_BASE + port_num * 0x400);

    HAL_GPIO_TogglePin(port, pin);
}
