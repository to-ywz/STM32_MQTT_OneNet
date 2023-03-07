/*
*********************************************************************************************************
*
*	模块名称 : SPI总线驱动
*	文件名称 : bsp_spi_bus.h
*
*********************************************************************************************************
*/

#ifndef __BSP_SPI_BUS_H
#define __BSP_SPI_BUS_H

// #define SOFT_SPI  /* 定义此行表示使用GPIO模拟SPI接口 */
#define HARD_SPI /* 定义此行表示使用CPU的硬件SPI接口 */

#include "stm32f4xx_hal.h"
#include "spi.h"
/*
    STM32硬件SPI接口 = SPI3 或者 SPI1
    由于SPI1的时钟源是90M, SPI3的时钟源是45M。为了获得更快的速度，软件上选择SPI1。
*/
#ifdef SOFT_SPI /* 软件SPI */
/* 定义GPIO端口 */
#define SPI_SCK_PORT_ENABLE() __HAL_RCC_GPIOF_CLK_ENABLE()
#define PORT_SCK GPIOF
#define PIN_SCK GPIO_PIN_7

#define SPI_MOSI_PORT_ENABLE() __HAL_RCC_GPIOF_CLK_ENABLE()
#define PORT_MOSI GPIOF
#define PIN_MOSI GPIO_PIN_9

#define SPI_MISO_PORT_ENABLE() __HAL_RCC_GPIOF_CLK_ENABLE()
#define PORT_MISO GPIOF
#define PIN_MISO GPIO_PIN_8

#define SCK_0() (PORT_SCK->BSRR |= PIN_SCK << 16)
#define SCK_1() (PORT_SCK->BSRR |= PIN_SCK)

#define MOSI_0() (PORT_MOSI->BSRR |= PIN_MOSI << 16)
#define MOSI_1() (PORT_MOSI->BSRR |= PIN_MOSI)

#define MISO_IS_HIGH() (HAL_GPIO_ReadPin(PORT_MISO, PIN_MISO) == GPIO_PIN_SET)
#endif

#ifdef HARD_SPI
#define SPI_HARD hspi1
#define RCC_SPI RCC_APB2Periph_SPI1

/* SPI or I2S mode selection masks */
#define SPI_Mode_Select ((uint16_t)0xF7FF)
#define I2S_Mode_Select ((uint16_t)0x0800)

/* SPI registers Masks */
#define CR1_CLEAR_Mask ((uint16_t)0x3040)
#define I2SCFGR_CLEAR_Mask ((uint16_t)0xF040)

/* SPI SPE mask */
#define CR1_SPE_Set ((uint16_t)0x0040)
#define CR1_SPE_Reset ((uint16_t)0xFFBF)
#endif
/*
    【SPI时钟最快是2分频，不支持不分频】
    如果是SPI1，2分频时SCK时钟 = 42M，4分频时SCK时钟 = 21M
    如果是SPI3, 2分频时SCK时钟 = 21M
*/
#define SPI_SPEED_42M SPI_BaudRatePrescaler_2
#define SPI_SPEED_21M SPI_BaudRatePrescaler_4
#define SPI_SPEED_5_2M SPI_BaudRatePrescaler_8
#define SPI_SPEED_2_6M SPI_BaudRatePrescaler_16
#define SPI_SPEED_1_3M SPI_BaudRatePrescaler_32
#define SPI_SPEED_0_6M SPI_BaudRatePrescaler_64

void bsp_initspibus(void);
void bsp_spiwrite0(uint8_t _ucByte);
uint8_t bsp_spiread0(void);
uint8_t bsp_spi_readwrite0(uint8_t _ucByte);

void bsp_spiwrite1(uint8_t _ucByte);
uint8_t bsp_spiread1(void);
uint8_t bsp_spi_readwrite1(uint8_t _ucByte);

void bsp_spibusenter(void);
void bsp_spibusexit(void);
uint8_t bsp_spibusbusy(void);

void bsp_setspisck(uint8_t _data);
#ifdef HARD_SPI /* 硬件SPI */
void bsp_spi_init();
#endif

#endif
