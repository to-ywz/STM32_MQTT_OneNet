

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
/* 如果需要 单独扩大可以 在bsp_uart中 自行设定串口*/
#define UART_BAUD 115200
#define UART_TX_BUF_SIZE 1 * 256
#define UART_RX_BUF_SIZE 1 * 256

/* 定义端口号 */
typedef enum
{
    COM1 = 0, /* USART1 */
    COM2 = 1, /* USART2 */
    COM3 = 2, /* USART3 */
    COM4 = 3, /* UART4 */
    COM5 = 4, /* UART5 */
    COM6 = 5, /* USART6 */
} COM_PORT_E;

typedef struct uart_buf
{
    uint8_t *buf;
    uint16_t bufsize;         /*  缓冲区大小 */
    __IO uint32_t write : 10; /*  缓冲区写指针 */
    __IO uint32_t read : 10;  /*  缓冲区读指针 */
    __IO uint32_t count : 10; /*  数据个数 */
} uart_buf_t;

/* 串口设备结构体 */
typedef struct
{
    USART_TypeDef *uart; /* STM32内部串口设备指针 */

    uart_buf_t txbuf;  /*发送缓存区*/
    uart_buf_t rxbuf; /*接受缓存区*/

    void (*before_send)(void);         /* 开始发送之前的回调函数指针（主要用于RS485切换到发送模式） */
    void (*over_send)(void);           /* 发送完毕的回调函数指针（主要用于RS485将发送模式切换为接收模式） */
    void (*new_recive)(uint8_t _byte); /* 串口收到数据的回调函数指针 */
    uint8_t sending;                   /* 正在发送中 */
} UART_T;

void bsp_com_init(COM_PORT_E com);
void com_databuf_send(COM_PORT_E com, uint8_t *buf, uint16_t len);
void com_char_send(COM_PORT_E com, uint8_t byte);
uint8_t com_getchar(COM_PORT_E com, uint8_t *byte);
void com_txfifo_clear(COM_PORT_E com);
void com_baudrate_setup(COM_PORT_E com, uint32_t baudrate);

void com_param_setup(COM_PORT_E com, uint32_t baudrate, uint32_t parity, uint32_t mode);
uint8_t com_tx_isempty(COM_PORT_E com);

#endif
