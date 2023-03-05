#ifndef _NRF24L01_OPS_H
#define _NRF24L01_OPS_H

#include <stdint.h>

/*定义NRF24L01错误枚举*/
typedef enum NRF24L01Error
{
    NRF24L01_NoError,
    NRF24L01_InitError,
    NRF24L01_Absent
} NRF24L01ErrorType;

/* 定义片选信号枚举 */
typedef enum NRF24L01CS
{
    NRF24L01CS_Enable,
    NRF24L01CS_Disable
} NRF24L01CSType;

/* 定义使能信号枚举 */
typedef enum NRF24L01CE
{
    NRF24L01CE_Enable,
    NRF24L01CE_Disable
} NRF24L01CEType;

/*定义模式枚举*/
typedef enum NRF24L01Mode
{
    NRF24L01TxMode = 0,
    NRF24L01RxMode = 1
} NRF24L01ModeType;

/* 定义NRF24L01对象类型 */
typedef struct NRF24L01Object
{
    uint8_t reg[8];                           // 记录前8个配置寄存器
    uint8_t (*ReadWriteByte)(uint8_t TxData); // 声明向nRF24L01读写一个字节的函数
    void (*ChipSelect)(NRF24L01CSType cs);    // 声明片选操作函数
    void (*ChipEnable)(NRF24L01CEType en);    // 声明使能及模式操作函数
    uint8_t (*GetIRQ)(void);                  // 声明中断获取函数
    void (*Delayms)(volatile uint32_t nTime); // 毫秒延时操作指针
} NRF24L01ObjectType;

typedef uint8_t (*NRF24L01ReadWriteByte_t)(uint8_t TxData); // 声明向nRF24L01读写一个字节的函数
typedef void (*NRF24L01ChipSelect_t)(NRF24L01CSType cs);    // 声明片选操作函数
typedef void (*NRF24L01ChipEnable_t)(NRF24L01CEType en);    // 声明使能及模式操作函数
typedef uint8_t (*NRF24L01GetIRQ_t)(void);                  // 声明中断获取函数
typedef void (*NRF24L01Delayms_t)(volatile uint32_t nTime); // 毫秒延时操作指针

/*启动NRF24L01发送一次数据包*/
uint8_t NRF24L01TransmitPacket(NRF24L01ObjectType *nrf, uint8_t *txbuf);

/*启动NRF24L01接收一次数据包*/
uint8_t NRF24L01ReceivePacket(NRF24L01ObjectType *nrf, uint8_t *rxbuf);

/*nRF24L01对象初始化函数*/
NRF24L01ErrorType NRF24L01Initialization(NRF24L01ObjectType *nrf,            // nRF24L01对象
                                         NRF24L01ReadWriteByte_t spiReadWrite, // SPI读写函数指针
                                         NRF24L01ChipSelect_t cs,              // 片选信号操作函数指针
                                         NRF24L01ChipEnable_t ce,              // 使能信号操作函数指针
                                         NRF24L01GetIRQ_t irq,                 // 中断信号获取函数指针
                                         NRF24L01Delayms_t delayms             // 毫秒延时
);

#endif

/*********** (C) COPYRIGHT 1999-2018 Moonan Technology *********END OF FILE****/
