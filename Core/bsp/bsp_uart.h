/**
 * @file bsp_uart.h
 * @author  秦殇 (you@domain.com)
 * @brief UART + FIFO
 * @version 0.1
 * @date 2020-07-08
 *
 * @copyright Copyright (c) 2020
 *
 */
#ifndef BSP_USART_FIFO_H_
#define BSP_USART_FIFO_H_

#include "stm32f4xx.h"
#include <stdio.h>
#include "bsp_gpio.h"
#include "ringbuffer.h"

#define MACROSTR(k) #k

#define UART_STATUS                     \
    X(UART_IDEL)     /* 空闲 */       \
    X(UART_REVICING) /* 接受数据 */ \
    X(UART_REVICED)  /* 接受完毕 */ \
    X(UART_HANDING)  /* 处理数据 */

// LED 工作状态枚举
typedef enum
{
#define X(Enum) Enum,
    UART_STATUS
#undef X
} Uart_Status_t;

#undef RX_STATUS

typedef struct
{
    Uart_Status_t status;      //   串口状态
    UART_HandleTypeDef *huart; //   串口实体
} UartInfo_t;

typedef struct UartObject
{
    uint8_t rx_Count;

    UartInfo_t info;         // 串口相关信息
    ringbuffer_t *rx_Buffer; // 循环队列
} UartObject_t;

/**
 * 串口宏定义，不同的串口挂载的总线和IO不一样，移植时需要修改这几个宏
 */
#define USING_RS485 0

#if USING_RS485
#define RS485_RX_EN() set_pin_low(RS485_RE_Port, RS485_RE_Pin)
#define RS485_TX_EN() set_pin_hig(RS485_RE_Port, RS485_RE_Pin)
#endif

//---------------------------- 接口函数 ------------------------------------
void uart_init(void);
void UARTx_SendData(UART_HandleTypeDef *huart, uint8_t *buf, uint16_t length);
void UARTx_printf(UART_HandleTypeDef *huart, char *Data, ...);

typedef enum
{
    COM1 = 0, /* USART1 */
    COM2 = 1, /* USART2 */
    COM3 = 2, /* USART3 */
    COM4 = 3, /* UART4 */
    COM5 = 4, /* UART5 */
    COM6 = 5, /* USART6 */
    // COM7 = 6, /* UART7 */
    // COM8 = 7  /* UART8 */
} com_port_t;

/* 串口设备结构体 */
typedef struct
{
    USART_TypeDef *uart; /* STM32内部串口设备指针 */

    ringbuffer_t rx_rb; /* 接收队列*/
    ringbuffer_t tx_rb; /* 发送队列*/

    void (*sendbefor)(void);          /* 开始发送之前的回调函数指针（主要用于RS485切换到发送模式） */
    void (*sendover)(void);           /* 发送完毕的回调函数指针（主要用于RS485将发送模式切换为接收模式） */
    void (*recivenew)(uint8_t _byte); /* 串口收到数据的回调函数指针 */
    uint8_t sending;                  /* 正在发送中 */
} UART_T;

void bsp_inituart(void);
void comsendbuf(com_port_t _ucPort, uint8_t *_ucaBuf, uint16_t _usLen);
void comsendchar(com_port_t _ucPort, uint8_t _ucByte);
uint8_t comgetchar(com_port_t _ucPort, uint8_t *_pByte);
void comcleartxfifo(com_port_t _ucPort);
void comsetbaud(com_port_t _ucPort, uint32_t _BaudRate);

void bsp_setuartparam(USART_TypeDef *Instance, uint32_t BaudRate, uint32_t Parity, uint32_t Mode);
uint8_t uarttxempty(com_port_t _ucPort);

#endif // !BSP_USART_FIFO_H_

//===================================END OF FILE ================================
