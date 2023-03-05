/**
 * @file bsp_uart.c
 * @author BlackSheep (blacksheep.208h@gmail.com)
 * @brief   采用串口中断+FIFO
 *          实现多个串口的同时访问
 *          后期考虑加入DMA 实现乒乓
 * @version 0.1
 * @date 2023-03-01
 *
 * @copyright Copyright (c) 2023
 *
 */
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include "bsp_gpio_def.h"
#include "bsp_uart.h"
#include "sys.h"

#define UART_NUM 1

#define ENABLE_INT() __set_PRIMASK(0)  /* 使能全局中断 */
#define DISABLE_INT() __set_PRIMASK(1) /* 禁止全局中断 */

struct uart_hw_info
{
    uint8_t tx_pin; // 发送引脚
    uint8_t rx_pin; // 接受引脚
    IRQn_Type iqr;  // 中断源
} uart_info[UART_NUM] = {
    {9, 10, USART1_IRQn}
    /*
    {53, 54, USART2_IRQn},
    {26, 27, USART3_IRQn},
    {42, 43, UART4_IRQn},
    {44, 50, UART5_IRQn},
    {38, 39, USART6_IRQn},*/
};

/* 定义每个串口结构体变量 */
static UART_T uart_list[UART_NUM];
#if UART1_FIFO_EN
static uint8_t txbuf_arr[UART_NUM][UART_TX_BUF_SIZE]; /* 发送缓冲区 */
static uint8_t rxbuf_arr[UART_NUM][UART_RX_BUF_SIZE]; /* 接收缓冲区 */
#endif

#if UART2_FIFO_EN
static uint8_t txBuf2[UART2_TX_BUF_SIZE]; /* 发送缓冲区 */
static uint8_t rxBuf2[UART2_RX_BUF_SIZE]; /* 接收缓冲区 */
#endif

static USART_TypeDef *usart_list[] = {
    USART1, USART2, USART3, UART4, UART5, USART6};

static void uart_hard_init(COM_PORT_E com);
static void uar_object_init(COM_PORT_E prot);
static USART_TypeDef *com_to_usart(COM_PORT_E com);
static UART_T *com_to_uart(COM_PORT_E com);

/**
 * @brief           配置串口的硬件参数
 *
 * @param com      串口编号
 * @param baudrate  波特率
 * @param parity    校验类型
 * @param mode      串口工作模式
 */
void com_param_setup(COM_PORT_E com, uint32_t baudrate, uint32_t parity, uint32_t mode)
{
    UART_HandleTypeDef UartHandle;

    /**
     * 配置串口硬件
     * 参数 异步串口模式 (UART Mode)
     * 配置如下:
     *  - 字长    = 8 位
     * - 停止位  = 1 个停止位
     * - 校验    = 参数Parity
     * - 波特率  = 参数BaudRate
     * - 硬件流控制关闭 (RTS and CTS signals)
     * */

    UartHandle.Instance = com_to_usart(com);

    UartHandle.Init.BaudRate = baudrate;
    UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
    UartHandle.Init.StopBits = UART_STOPBITS_1;
    UartHandle.Init.Parity = parity;
    UartHandle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    UartHandle.Init.Mode = mode;
    UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&UartHandle) != HAL_OK)
    {
    }
}

/**
 * @brief 串口初始化
 *
 */
void bsp_com_init(COM_PORT_E com)
{

    uar_object_init(com); /* 必须先初始化全局变量,再配置硬件 */
    uart_hard_init(com);  /* 配置串口的硬件参数(波特率等) */
}

/**
 * @brief   初始化串口逻辑
 *
 */
static void uar_object_init(COM_PORT_E com)
{
    uart_list[com].uart = USART1;                    /* STM32 串口设备 */
    uart_list[com].txbuf.buf = txbuf_arr[com];       /* 发送缓冲区指针 */
    uart_list[com].rxbuf.buf = rxbuf_arr[com];       /* 接收缓冲区指针 */
    uart_list[com].txbuf.bufsize = UART_TX_BUF_SIZE; /* 发送缓冲区大小 */
    uart_list[com].rxbuf.bufsize = UART_RX_BUF_SIZE; /* 接收缓冲区大小 */
    uart_list[com].txbuf.write = 0;                  /* 发送FIFO写索引 */
    uart_list[com].txbuf.read = 0;                   /* 发送FIFO读索引 */
    uart_list[com].rxbuf.write = 0;                  /* 接收FIFO写索引 */
    uart_list[com].rxbuf.read = 0;                   /* 接收FIFO读索引 */
    uart_list[com].txbuf.count = 0;                  /* 待发送的数据个数 */
    uart_list[com].rxbuf.count = 0;                  /* 接收到的新数据个数 */
    uart_list[com].before_send = 0;                  /* 发送数据前的回调函数 */
    uart_list[com].over_send = 0;                    /* 发送完毕后的回调函数 */
    uart_list[com].new_recive = 0;                   /* 接收到新数据后的回调函数 */
    uart_list[com].sending = 0;                      /* 正在发送中标志 */
}

/**
 * @brief       使能 UART 时钟
 *
 * @param com  端口
 */
static inline void uart_clk_enable(COM_PORT_E com)
{
    switch (com)
    {
    case COM1:
        __HAL_RCC_USART1_CLK_ENABLE();
        break;
    case COM2:
        __HAL_RCC_USART2_CLK_ENABLE();
        break;
    case COM3:
        __HAL_RCC_USART3_CLK_ENABLE();
        break;
    case COM4:
        __HAL_RCC_UART4_CLK_ENABLE();
        break;
    case COM5:
        __HAL_RCC_UART5_CLK_ENABLE();
        break;
    case COM6:
        __HAL_RCC_USART6_CLK_ENABLE();
        break;

    default:
        break;
    }
}

/**
 * @brief   串口初始化
 *
 * @note    串口参数通过bsp_setuartparam来配置
 */
static void uart_hard_init(COM_PORT_E com)
{
    uint16_t tx_pin = 0, rx_pin = 0;
    uint8_t port_num = 0;
    GPIO_TypeDef *port = 0;
    GPIO_InitTypeDef gpio_init_structure = {};

    tx_pin = 1U << GET_PIN(uart_info[com].tx_pin);
    port_num = GET_PORT(uart_info[com].tx_pin);
    port = (GPIO_TypeDef *)(GPIOA_BASE + port_num * 0x400);

    /* 使能 GPIO TX/RX 时钟 */
    RCC->AHB1ENR |= 1 << port_num;

    /* 使能 USARTx 时钟 */
    uart_clk_enable(com);

    /* 配置TX引脚 */
    gpio_init_structure.Pin = tx_pin;
    gpio_init_structure.Mode = GPIO_MODE_AF_PP;
    gpio_init_structure.Pull = GPIO_PULLUP;
    gpio_init_structure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio_init_structure.Alternate = (com < 3 ? 0x07 : 0x08);
    HAL_GPIO_Init(port, &gpio_init_structure);

    /* 配置RX引脚 */
    tx_pin = 1U << GET_PIN(uart_info[com].rx_pin);
    gpio_init_structure.Pin = rx_pin;
    HAL_GPIO_Init(port, &gpio_init_structure);

    /* 配置NVIC the NVIC for UART */
    HAL_NVIC_SetPriority(uart_info[com].iqr, 0, 1);
    HAL_NVIC_EnableIRQ(uart_info[com].iqr);

    /* 配置波特率、奇偶校验 */
    com_param_setup(com, UART_BAUD, UART_PARITY_NONE, UART_MODE_TX_RX);

    CLEAR_BIT(com_to_usart(com)->SR, USART_SR_TC);   /* 清除TC发送完成标志 */
    CLEAR_BIT(com_to_usart(com)->SR, USART_SR_RXNE); /* 清除RXNE接收标志 */
    // USART_CR1_PEIE | USART_CR1_RXNEIE
    SET_BIT(com_to_usart(com)->CR1, USART_CR1_RXNEIE); /* 使能PE. RX接受中断 */

#if UART1_FIFO_EN /* 串口1 */

#endif
#if UART2_FIFO_EN /* 串口2 */
    /* 使能 GPIO TX/RX 时钟 */
    USART2_TX_GPIO_CLK_ENABLE();
    USART2_RX_GPIO_CLK_ENABLE();

    /* 使能 USARTx 时钟 */
    USART2_CLK_ENABLE();

    /* 配置TX引脚 */
    GPIO_InitStruct.Pin = USART2_TX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = USART2_TX_AF;
    HAL_GPIO_Init(USART2_TX_GPIO_PORT, &GPIO_InitStruct);

    /* 配置RX引脚 */
    GPIO_InitStruct.Pin = USART2_RX_PIN;
    GPIO_InitStruct.Alternate = USART2_RX_AF;
    HAL_GPIO_Init(USART2_RX_GPIO_PORT, &GPIO_InitStruct);

    /* 配置NVIC the NVIC for UART */
    HAL_NVIC_SetPriority(USART2_IRQn, 0, 1);
    HAL_NVIC_EnableIRQ(USART2_IRQn);

    /* 配置波特率、奇偶校验 */
    bsp_uart_param_setup(USART2, UART2_BAUD, UART_PARITY_NONE, UART_MODE_TX_RX);

    CLEAR_BIT(USART2->SR, USART_SR_TC);   /* 清除TC发送完成标志 */
    CLEAR_BIT(USART2->SR, USART_SR_RXNE); /* 清除RXNE接收标志 */
    // USART_CR1_PEIE | USART_CR1_RXNEIE
    SET_BIT(USART2->CR1, USART_CR1_RXNEIE); /* 使能PE. RX接受中断 */
#endif
}

/**
 * @brief           将COM端口号转换为UART指针
 *
 * @param com      端口号(COM1 - COM6)
 * @retval          uart指针
 */
static inline UART_T *com_to_uart(COM_PORT_E com)
{
    return &(uart_list[com]);
}

/**
 * @brief           将COM端口号转换为 USART_TypeDef* USARTx
 *
 * @param com      COM编号
 * @retval          USART_TypeDef*,  USART1, USART2, USART3, UART4, UART5,USART6
 */
static inline USART_TypeDef *com_to_usart(COM_PORT_E com)
{
    return usart_list[com];
}

/**
 * @brief       清零串口发送缓冲区
 *
 * @param com  端口号(COM1 - COM6)
 */
void com_txfifo_clear(COM_PORT_E com)
{
    UART_T *uart;

    uart = com_to_uart(com);
    if (uart == 0)
    {
        return;
    }

    uart->txbuf.write = 0;
    uart->txbuf.read = 0;
    uart->txbuf.count = 0;
}

/**
 * @brief           清零串口接收缓冲区
 *
 * @param com      端口号(COM1 - COM6)
 */
static void com_rxfifo_clear(COM_PORT_E com)
{
    UART_T *uart;

    uart = com_to_uart(com);
    if (uart == 0)
    {
        return;
    }

    uart->rxbuf.write = 0;
    uart->rxbuf.read = 0;
    uart->rxbuf.count = 0;
}

/**
 * @brief           设置串口的波特率
 *
 * @param com      端口号(COM1 - COM6)
 * @param baud_rate 波特率,8倍过采样  波特率.0-12.5Mbps
 *                  16倍过采样 波特率.0-6.25Mbps
 * @note            固定设置为无校验,收发都使能模式
 */
void com_baudrate_setup(COM_PORT_E com, uint32_t baud_rate)
{
    USART_TypeDef *USARTx;

    USARTx = com_to_usart(com);
    if (USARTx == 0)
    {
        return;
    }

    com_param_setup(com, baud_rate, UART_PARITY_NONE, UART_MODE_TX_RX);
}

/*
*********************************************************************************************************
*串口数据接收
*********************************************************************************************************
*/
/**
 * @brief       填写数据到UART发送缓冲区,并启动发送中
 *
 * @param uart  串口指针
 * @param buf   要发送的数据
 * @param len   数据大小
 */
static void uart_databuf_send(UART_T *uart, uint8_t *buf, uint16_t len)
{
    uint16_t i;

    for (i = 0; i < len; i++)
    {
        /* 如果发送缓冲区已经满了,则等待缓冲区空 */
        while (1)
        {
            __IO uint16_t usCount;

            DISABLE_INT();
            usCount = uart->txbuf.count;
            ENABLE_INT();

            if (usCount < uart->txbuf.bufsize)
            {
                break;
            }
            else if (usCount == uart->txbuf.bufsize) /* 数据已填满缓冲区 */
            {
                if ((uart->uart->CR1 & USART_CR1_TXEIE) == 0)
                {
                    SET_BIT(uart->uart->CR1, USART_CR1_TXEIE);
                }
            }
        }

        /* 将新数据填入发送缓冲区 */
        uart->txbuf.buf[uart->txbuf.write] = buf[i];

        DISABLE_INT();
        if (++uart->txbuf.write >= uart->txbuf.bufsize)
        {
            uart->txbuf.write = 0;
        }
        uart->txbuf.count++;
        ENABLE_INT();
    }

    SET_BIT(uart->uart->CR1, USART_CR1_TXEIE); /* 使能发送中断（缓冲区空） */
}

/**
 * @brief       向串口发送一组数据
 *
 * @param com  端口号(COM1 - COM8)
 * @param buf   待发送的数据缓冲区
 * @param len   数据长度
 * @note        数据放到发送缓冲区后立即返回
 */
void com_databuf_send(COM_PORT_E com, uint8_t *buf, uint16_t len)
{
    UART_T *uart;

    uart = com_to_uart(com);
    if (uart == 0)
    {
        return;
    }

    if (uart->before_send != 0)
    {
        uart->before_send(); /* 如果是RS485通信,可以在这个函数中将RS485设置为发送模式 */
    }

    uart_databuf_send(com_to_uart(com), buf, len);
}

/**
 * @brief       向串口发送1个字节
 *
 * @param com  端口号(COM1 - COM8)
 * @param byte  待发送的数据
 * @note        数据放到发送缓冲区后立即返回,由中断服务程序在后台完成发送
 */
void com_send_char(COM_PORT_E com, uint8_t byte)
{
    com_databuf_send(com, &byte, 1);
}

/*
*********************************************************************************************************
串口数据接收
*********************************************************************************************************
*/
/**
 * @brief       判断发送缓冲区是否为空
 *
 * @param com  串口设备
 * @retval      1:空
 *              0:为不空
 */
uint8_t com_tx_isempty(COM_PORT_E com)
{
    UART_T *uart;
    uint8_t sending;

    uart = com_to_uart(com);
    if (uart == 0)
    {
        return 0;
    }

    sending = uart->sending;

    if (sending != 0)
    {
        return 0;
    }
    return 1;
}

/**
 * @brief           从串口接收缓冲区读取1字节数据
 *
 * @param uart      串口设备
 * @param byte      存放读取数据的指针
 * @retval          0:  表示无数据
 *                  1:  读取到有效字节
 */
static uint8_t uart_getchar(UART_T *uart, uint8_t *byte)
{
    uint16_t usCount;

    /* usRxWrite 变量在中断函数中被改写,主程序读取该变量时,必须进行临界区保护 */
    DISABLE_INT();
    usCount = uart->rxbuf.count;
    ENABLE_INT();

    /* 如果读和写索引相同,则返回0 */
    // if (uart->usRxRead == usRxWrite)
    if (usCount == 0) /* 已经没有数据 */
    {
        return 0;
    }
    else
    {
        *byte = uart->rxbuf.buf[uart->rxbuf.read]; /* 从串口接收FIFO取1个数据 */

        /* 改写FIFO读索引 */
        DISABLE_INT();
        if (++uart->rxbuf.read >= uart->rxbuf.bufsize)
        {
            uart->rxbuf.read = 0;
        }
        uart->rxbuf.count--;
        ENABLE_INT();
        return 1;
    }
}

/**
 * @brief           从接收缓冲区读取1字节,非阻塞
 *
 * @param com      端口号(COM1 - COM6)
 * @param byte      接收到的数据存放在这个地址
 * @retval          0:  表示无数据
 *                  1:  读取到有效字节
 */
uint8_t com_getchar(COM_PORT_E com, uint8_t *byte)
{
    UART_T *uart;

    uart = com_to_uart(com);
    if (uart == 0)
    {
        return 0;
    }

    return uart_getchar(uart, byte);
}

/**
 * @brief       中断
 *
 * @param uart  串口指针
 */
static void uart_isr(UART_T *uart)
{
    uint32_t isrflags = READ_REG(uart->uart->SR);
    uint32_t cr1its = READ_REG(uart->uart->CR1);
    uint32_t cr3its = READ_REG(uart->uart->CR3);

    /* 处理接收中断  */
    if ((isrflags & USART_SR_RXNE) != RESET)
    {
        /* 从串口接收数据寄存器读取数据存放到接收FIFO */
        uint8_t ch;

        ch = READ_REG(uart->uart->DR);                  /* 读取串口接收寄存器 */
        uart->rxbuf.buf[uart->rxbuf.write] = ch;        /* 填入串口接收FIFO */
        if (++uart->rxbuf.write >= uart->rxbuf.bufsize) /* 接收 FIFO 的写指针+1 */
        {
            uart->rxbuf.write = 0;
        }
        if (uart->rxbuf.count < uart->rxbuf.bufsize) /* 统计未处理的字节个数 */
        {
            uart->rxbuf.count++;
        }

        /* 回调函数,通知应用程序收到新数据,一般是发送1个消息或者设置一个标记 */
        {
            if (uart->new_recive)
            {
                uart->new_recive(ch); /* 比如,交给MODBUS解码程序处理字节流 */
            }
        }
    }

    /* 处理发送缓冲区空中断 */
    if (((isrflags & USART_SR_TXE) != RESET) && (cr1its & USART_CR1_TXEIE) != RESET)
    {
        if (uart->txbuf.count == 0)
        {
            /* 发送缓冲区的数据已取完时, 禁止发送缓冲区空中断 （注意：此时最后1个数据还未真正发送完毕）*/
            // USART_ITConfig(uart->uart, USART_IT_TXE, DISABLE);
            CLEAR_BIT(uart->uart->CR1, USART_CR1_TXEIE);

            /* 使能数据发送完毕中断 */
            // USART_ITConfig(uart->uart, USART_IT_TC, ENABLE);
            SET_BIT(uart->uart->CR1, USART_CR1_TCIE);
        }
        else
        {
            uart->sending = 1;

            /* 从发送FIFO取1个字节写入串口发送数据寄存器 */
            // USART_Senddata(uart->uart, uart->txbuf.buf[uart->usTxRead]);
            uart->uart->DR = uart->txbuf.buf[uart->txbuf.read];
            if (++uart->txbuf.read >= uart->txbuf.bufsize)
            {
                uart->txbuf.read = 0;
            }
            uart->txbuf.count--;
        }
    }
    /* 数据bit位全部发送完毕的中断 */
    if (((isrflags & USART_SR_TC) != RESET) && ((cr1its & USART_CR1_TCIE) != RESET))
    {
        // if (uart->usTxRead == uart->usTxWrite)
        if (uart->txbuf.count == 0)
        {
            /* 如果发送FIFO的数据全部发送完毕,禁止数据发送完毕中断 */
            // USART_ITConfig(uart->uart, USART_IT_TC, DISABLE);
            CLEAR_BIT(uart->uart->CR1, USART_CR1_TCIE);

            /* 回调函数, 一般用来处理RS485通信,将RS485芯片设置为接收模式,避免抢占总线 */
            if (uart->over_send)
            {
                uart->over_send();
            }

            uart->sending = 0;
        }
        else
        { // 异常处理
            /* 正常情况下,不会进入此分支 */

            /* 如果发送FIFO的数据还未完毕,则从发送FIFO取1个数据写入发送数据寄存器 */
            // USART_Senddata(uart->uart, uart->txbuf.buf[uart->usTxRead]);
            uart->uart->DR = uart->txbuf.buf[uart->txbuf.read];
            if (++uart->txbuf.read >= uart->txbuf.bufsize)
            {
                uart->txbuf.read = 0;
            }
            uart->txbuf.count--;
        }
    }
}

/*
*********************************************************************************************************
 USART1_IRQHandler  USART2_IRQHandler USART3_IRQHandler UART4_IRQHandler UART5_IRQHandler等
 UART中断服务程序
*********************************************************************************************************
*/
#if UART1_FIFO_EN
void USART1_IRQHandler(void)
{
    uart_isr(&uart_list[0]);
}
#endif
#if UART2_FIFO_EN
void USART2_IRQHandler(void)
{
    uart_isr(&uart[1]);
}
#endif

/**
 * @brief           格式化输出，类似于C库中的printf
 *
 * @param com       串口编号
 * @param data      格式化字符串(类似printf)
 * @param ...       其他参数
 */
static uint8_t buffer[2] = {0x0d, 0x0a};
void com_printf(COM_PORT_E com, char *data, ...)
{
    const char *s;
    int d;
    char buf[16];

    va_list ap;
    va_start(ap, data);

    while (*data != 0) // 判断是否到达字符串结束符
    {
        if (*data == 0x5c) //'\'
        {
            switch (*++data)
            {
            case 'r': // 回车符
                com_send_char(com, buffer[0]);
                data++;
                break;

            case 'n': // 换行符
                com_send_char(com, buffer[1]);
                data++;
                break;

            default:
                data++;
                break;
            }
        }

        else if (*data == '%')
        { //
            switch (*++data)
            {
            case 's': // 字符串
                s = va_arg(ap, const char *);

                for (; *s; s++)
                {
                    com_send_char(com, *s);
                    // while (!(READ_BIT(com_to_usart(com)->SR, USART_SR_TXE) == (USART_SR_TXE)))
                    //     ;
                }

                data++;

                break;

            case 'd':
                // 十进制
                d = va_arg(ap, int);

                itoa(d, buf, 10);

                for (s = buf; *s; s++)
                {
                    com_send_char(com, *s);
                    // while (!(READ_BIT(com_to_usart(com)->SR, USART_SR_TXE) == (USART_SR_TXE)))
                    //     ;
                }

                data++;

                break;

            default:
                data++;

                break;
            }
        }

        else
        {
            com_send_char(com, *data);
            data++;
        }
        // while (!(READ_BIT(com_to_usart(com)->SR, USART_SR_TXE) == (USART_SR_TXE)))
        //     ;
    }
}

/*
*********************************************************************************************************
C库重载
*********************************************************************************************************
*/
/**
 * @brief   重定义putc函数
 *
 */
int fputc(int ch, FILE *f)
{
    /* 将需要printf的字符通过串口中断FIFO发送出去,printf函数会立即返回 */
    com_send_char(COM1, ch);

    return ch;
}

/**
 * @brief       重定义getc函数
 *              用于 getchar函数从串口获取一个数据
 */
int fgetc(FILE *f)
{

    /* 从串口接收FIFO中取1个数据, 只有取到数据才返回 */
    uint8_t data;

    while (com_getchar(COM1, &data) == 0)
        ;

    return data;
}