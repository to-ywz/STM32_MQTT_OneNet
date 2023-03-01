

#ifndef _BSP_USART_FIFO_H_
#define _BSP_USART_FIFO_H_

#include "stm32f4xx_hal.h"

#define UART1_FIFO_EN 1
#define UART2_FIFO_EN 0
#define UART3_FIFO_EN 0
#define UART4_FIFO_EN 0
#define UART5_FIFO_EN 0
#define UART6_FIFO_EN 0
#define UART7_FIFO_EN 0
#define UART8_FIFO_EN 0

/* 定义串口波特率和FIFO缓冲区大小，分为发送缓冲区和接收缓冲区, 支持全双工 */
#if UART1_FIFO_EN
#define UART1_BAUD 115200
#define UART1_TX_BUF_SIZE 1 * 1024
#define UART1_RX_BUF_SIZE 1 * 1024
#endif
#if UART1_FIFO_EN
#define UART2_BAUD 115200
#define UART2_TX_BUF_SIZE 1 * 1024
#define UART2_RX_BUF_SIZE 1 * 1024
#endif
/* 定义端口号 */
typedef enum
{
    COM1 = 0, /* USART1 */
    COM2 = 1, /* USART2 */
    COM3 = 2, /* USART3 */
    COM4 = 3, /* UART4 */
    COM5 = 4, /* UART5 */
    COM6 = 5, /* USART6 */
    COM7 = 6, /* UART7 */
    COM8 = 7  /* UART8 */
} COM_PORT_E;
/* 串口设备结构体 */
typedef struct
{
    USART_TypeDef *uart;     /* STM32内部串口设备指针 */
    uint8_t *ptxbuf;         /* 发送缓冲区 */
    uint8_t *prxbuf;         /* 接收缓冲区 */
    uint16_t ustxbufsize;    /* 发送缓冲区大小 */
    uint16_t usRxbufsize;    /* 接收缓冲区大小 */
    __IO uint16_t tx_write; /* 发送缓冲区写指针 */
    __IO uint16_t tx_read;  /* 发送缓冲区读指针 */
    __IO uint16_t tx_count; /* 等待发送的数据个数 */

    __IO uint16_t rx_write; /* 接收缓冲区写指针 */
    __IO uint16_t rx_read;  /* 接收缓冲区读指针 */
    __IO uint16_t rx_count; /* 还未读取的新数据个数 */

    void (*before_send)(void);          /* 开始发送之前的回调函数指针（主要用于RS485切换到发送模式） */
    void (*over_send)(void);           /* 发送完毕的回调函数指针（主要用于RS485将发送模式切换为接收模式） */
    void (*new_recive)(uint8_t _byte); /* 串口收到数据的回调函数指针 */
    uint8_t sending;                  /* 正在发送中 */
} UART_T;

void bsp_com_init(COM_PORT_E port);
void com_buf_send(COM_PORT_E port, uint8_t *buf, uint16_t len);
void com_send_char(COM_PORT_E port, uint8_t byte);
uint8_t com_getchar(COM_PORT_E port, uint8_t *byte);
void com_txfifo_clear(COM_PORT_E port);
void com_set_baud(COM_PORT_E port, uint32_t baudrate);

void bsp_uart_param_setup(USART_TypeDef *instance, uint32_t baudrate, uint32_t parity, uint32_t mode);
uint8_t uart_tx_isempty(COM_PORT_E port);

#endif
