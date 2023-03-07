#ifndef _NRF24L01_OPS_H
#define _NRF24L01_OPS_H

#include <stdint.h>

/*定义NRF24L01错误枚举*/
typedef enum NRF24L01ERROR
{
    NRF24L01_NOERROR,
    NRF24L01_INITERROR,
    NRF24L01_ABSENT
} NRF24L01_ERROR_ET;

/* 定义片选信号枚举 */
typedef enum NRF24L01CS
{
    NRF24L01CS_ENABLE,
    NRF24L01CS_DISABLE
} NRF24L01_CS_ET;

/* 定义使能信号枚举 */
typedef enum NRF24L01CE
{
    NRF24L01CE_DISABLE,
    NRF24L01CE_ENABLE
} NRF24L01_CE_ET;

/*定义模式枚举*/
typedef enum NRF24L01_MODE
{
    NRF24L01_TXMODE = 0,
    NRF24L01_RXMODE = 1
} NRF24L01ModeType;

/* 定义NRF24L01对象类型 */
typedef struct nrf24l01_struct
{
    uint8_t reg[8];                           // 记录前8个配置寄存器
    uint8_t (*byte_readwrite)(uint8_t txbuf); // 声明向nRF24L01读写一个字节的函数
    void (*chip_select)(NRF24L01_CS_ET cs);   // 声明片选操作函数
    void (*chip_enable)(NRF24L01_CE_ET en);   // 声明使能及模式操作函数
    uint8_t (*get_iqr)(void);                 // 声明中断获取函数
    void (*delayms)(volatile uint16_t ntime); // 毫秒延时操作指针
} nrf24l01_t;

typedef uint8_t (*nrf24l01_byte_readwrite_t)(uint8_t txbuf); // 声明向nRF24L01读写一个字节的函数
typedef void (*nrf24l01_chip_select_t)(NRF24L01_CS_ET cs);   // 声明片选操作函数
typedef void (*nrf24l01_chip_enable_t)(NRF24L01_CE_ET en);   // 声明使能及模式操作函数
typedef uint8_t (*nrf24l01_get_iqr_t)(void);                 // 声明中断获取函数
typedef void (*nrf24l01_delayms_t)(volatile uint16_t ntime); // 毫秒延时操作指针

uint8_t nrf24l01_packet_xmit(nrf24l01_t *nrf, uint8_t *txbuf);
uint8_t nrf24l01_packet_recv(nrf24l01_t *nrf, uint8_t *rxbuf);
NRF24L01_ERROR_ET nrf24L01_init(nrf24l01_t *nrf,
                                nrf24l01_byte_readwrite_t spiReadWrite,
                                nrf24l01_chip_select_t cs,
                                nrf24l01_chip_enable_t ce,
                                nrf24l01_get_iqr_t irq,
                                nrf24l01_delayms_t delayms);

#endif

/*********** (C) COPYRIGHT 1999-2018 Moonan Technology *********END OF FILE****/
