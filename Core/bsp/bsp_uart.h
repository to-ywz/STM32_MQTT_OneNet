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

#if USING_RS485
#define RS485_RX_EN() set_pin_low(RS485_RE_Port, RS485_RE_Pin)
#define RS485_TX_EN() set_pin_hig(RS485_RE_Port, RS485_RE_Pin)
#endif

#define USART_RX_BUF_SIZE 128
#define USART_TX_BUF_SIZE 128
#define REC_LENGTH 1

extern uint8_t UARTFlag;
extern char dataString[10][10];
extern uint8_t pRxBuffer[USART_RX_BUF_SIZE];
extern uint8_t pTxBuffer[USART_TX_BUF_SIZE];
extern uint8_t rxCount;
extern uint8_t txCount;
extern uint8_t UARTx_temp[REC_LENGTH];


extern UartObject_t uart2;

//---------------------------- 接口函数 ------------------------------------
void uart_init(void);
void rxDataHandler(void);
void uart_sendByte(UART_HandleTypeDef *huart, uint8_t ch);
void uart_sendString(UART_HandleTypeDef *huart, char *str);
void USART_printf(UART_HandleTypeDef *USARTx, char *Data, ...);
uint8_t Board_UARTGetCharBlocking(void);

#endif // !BSP_USART_FIFO_H_

//===================================END OF FILE ================================
