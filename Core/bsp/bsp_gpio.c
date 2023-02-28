/**
 * @file bsp_gpio.c
 * @author BlackSheep (blacksheep.208h@gmail.com)
 * @brief   STM32 GPIO通用初始化
 * @version 0.1
 * @date 2023-02-07
 * @note 不建议将 外部中断服务函数在这里实现
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "stm32f4xx_hal.h"
#include "bsp_gpio.h"
#include "bsp_gpio_def.h"

/* 外部中断 0-15 pin */
static IRQn_Type pin_iqr_map[] = {
    EXTI0_IRQn,
    EXTI1_IRQn,
    EXTI2_IRQn,
    EXTI3_IRQn,
    EXTI4_IRQn,
    EXTI9_5_IRQn,
    EXTI9_5_IRQn,
    EXTI9_5_IRQn,
    EXTI9_5_IRQn,
    EXTI9_5_IRQn,
    EXTI15_10_IRQn,
    EXTI15_10_IRQn,
    EXTI15_10_IRQn,
    EXTI15_10_IRQn,
    EXTI15_10_IRQn,
    EXTI15_10_IRQn,
};

static void hw_pin_mode(uint16_t pin_num, PIN_MODE_ET mode);
static void hw_pin_write(uint16_t pin_num, uint8_t value);
static uint8_t hw_pin_read(uint16_t pin_num);
static void hw_pin_toggle(uint16_t pin_num);
static void hw_pin_init_irq(uint16_t pin_num, PIN_IQR_MODE_ET mode, uint8_t priority);

//=================================软件层次==========================================
/**
 * @brief       配置引脚的GPIO工作模式
 *
 * @param pin   要配置的引脚编号
 * @param mode  GPIO的工作模式
 */
void bsp_pin_mode(uint16_t pin_num, PIN_MODE_ET mode)
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

/**
 * @brief           配置外部中断
 *
 * @param pin_num   引脚号
 * @param mode      外部中断工作模式
 * @param priority  优先级
 * @note            优先级的取值范围为[0:15], 从大到小
 */
void bsp_pin_exit_init(uint16_t pin_num, PIN_IQR_MODE_ET mode, uint8_t priority)
{
    hw_pin_init_irq(pin_num, mode, priority);
}

//=================================硬件层次==========================================
/* 这里 可以实现多种MCU的初始化,减少代码大小可以使用宏来实现 */
/**
 * @brief       配置引脚的GPIO工作模式
 *
 * @param pin   要配置的引脚的编号
 * @param mode  GPIO的工作模式
 */
static void hw_pin_mode(uint16_t pin_num, PIN_MODE_ET mode)
{

    uint16_t pin = 0;
    uint8_t port_num = 0;
    GPIO_TypeDef *port = 0;
    GPIO_InitTypeDef gpio_init_structure = {};

    // 获取需要配置引脚的信息
    pin = 1U << GET_PIN(pin_num);
    port_num = GET_PORT(pin_num);
    port = (GPIO_TypeDef *)(GPIOA_BASE + port_num * 0x400);

    // * 使能对应时钟
    RCC->AHB1ENR |= 1U << port_num;

    /* 配置初始化结构体模式 */
    gpio_init_structure.Pin = pin;
    gpio_init_structure.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_init_structure.Pull = GPIO_NOPULL;
    gpio_init_structure.Speed = GPIO_SPEED_FREQ_HIGH;

    if (mode == PIN_MODE_OUTPUT)
    {
        /* 输出模式配置 */
        gpio_init_structure.Mode = GPIO_MODE_OUTPUT_PP;
        gpio_init_structure.Pull = GPIO_NOPULL;
    }
    else if (mode == PIN_MODE_INPUT)
    {
        /* 浮空输入模式 */
        gpio_init_structure.Mode = GPIO_MODE_INPUT;
        gpio_init_structure.Pull = GPIO_NOPULL;
    }
    else if (mode == PIN_MODE_INPUT_PULLUP)
    {
        /* 上拉输入模式 */
        gpio_init_structure.Mode = GPIO_MODE_INPUT;
        gpio_init_structure.Pull = GPIO_PULLUP;
    }
    else if (mode == PIN_MODE_INPUT_PULLDOWN)
    {
        /* 下拉输入模式 */
        gpio_init_structure.Mode = GPIO_MODE_INPUT;
        gpio_init_structure.Pull = GPIO_PULLDOWN;
    }
    else if (mode == PIN_MODE_OUTPUT_OD)
    {
        /* 开漏输出模式 */
        gpio_init_structure.Mode = GPIO_MODE_OUTPUT_OD;
        gpio_init_structure.Pull = GPIO_NOPULL;
    }

    HAL_GPIO_Init(port, &gpio_init_structure);
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

static void hw_pin_init_irq(uint16_t pin_num, PIN_IQR_MODE_ET mode, uint8_t priority)
{
    uint16_t pin = 0;
    uint8_t port_num = 0;
    GPIO_TypeDef *port = 0;
    GPIO_InitTypeDef gpio_init_structure = {};

    pin = 1U << GET_PIN(pin_num);
    port_num = GET_PORT(pin_num);
    port = (GPIO_TypeDef *)(GPIOA_BASE + port_num * 0x400);

    /* 使能对应时钟 */
    RCC->AHB1ENR |= 1U << port_num;

    gpio_init_structure.Pin = pin;
    gpio_init_structure.Speed = GPIO_SPEED_FREQ_HIGH;

    switch (mode)
    {
    case PIN_IRQ_MODE_RISING:
        gpio_init_structure.Pull = GPIO_PULLDOWN;
        gpio_init_structure.Mode = GPIO_MODE_IT_RISING;
        break;
    case PIN_IRQ_MODE_FALLING:
        gpio_init_structure.Pull = GPIO_PULLUP;
        gpio_init_structure.Mode = GPIO_MODE_IT_FALLING;
        break;
    case PIN_IRQ_MODE_RISING_FALLING:
        gpio_init_structure.Pull = GPIO_NOPULL;
        gpio_init_structure.Mode = GPIO_MODE_IT_RISING_FALLING;
        break;
    default:
        break;
    }
    HAL_GPIO_Init(port, &gpio_init_structure);
    HAL_NVIC_SetPriority(pin_iqr_map[GET_PIN(pin_num)], priority, 0);
    HAL_NVIC_EnableIRQ(pin_iqr_map[GET_PIN(pin_num)]);
}

//==================================================End=================================================
