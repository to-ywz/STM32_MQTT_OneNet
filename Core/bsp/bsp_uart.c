
#include <string.h>
#include "bsp_uart.h"
#include "usart.h"
#include "bsp_gpio_def.h"
#include "stm32f407xx.h"
#include <stdarg.h>
//------------------------------ 用户宏定义 ----------------------------------
#define UART_RX_BUF_SIZE 128
#define REC_LENGTH 1 // 串口一次接受数据大小, 建议为1
//------------------------------ 用户变量定义 --------------------------------

UartObject_t uart1;
static struct ringbuffer rx_rb;
static uint8_t uart_rx_temp[REC_LENGTH];
static uint8_t rx_buffer[UART_RX_BUF_SIZE];

// ==================COM==============================================

typedef struct uart_info_t
{
    USART_TypeDef *instance;
    uint32_t baund;
    uint8_t mode;
    uint8_t parity;

    /* gpio 信息*/
    struct
    {
        uint16_t rx_pin_num; // rx 引脚
        uint16_t tx_pin_num; // tx 引脚

        IRQn_Type iqr; // 中断源
    } gpio_info;

} uart_info_t;

static uart_info_t uart_info_arr[] = {
    {.instance = USART1, .baund = 115200, .mode = UART_MODE_TX_RX, .parity = UART_PARITY_NONE, {.iqr = USART1_IRQn, .rx_pin_num = 10, .tx_pin_num = 9}},
    {.instance = USART2, .baund = 115200, .mode = UART_MODE_TX_RX, .parity = UART_PARITY_NONE, {.iqr = USART2_IRQn, .rx_pin_num = 3, .tx_pin_num = 2}},
    {.instance = USART3, .baund = 115200, .mode = UART_MODE_TX_RX, .parity = UART_PARITY_NONE, {.iqr = USART3_IRQn, .rx_pin_num = 27, .tx_pin_num = 26}},
    {.instance = UART4, .baund = 115200, .mode = UART_MODE_TX_RX, .parity = UART_PARITY_NONE, {.iqr = UART4_IRQn, .rx_pin_num = 43, .tx_pin_num = 42}},
    {.instance = UART5, .baund = 115200, .mode = UART_MODE_TX_RX, .parity = UART_PARITY_NONE, {.iqr = UART5_IRQn, .rx_pin_num = 50, .tx_pin_num = 44}},
    {.instance = USART6, .baund = 115200, .mode = UART_MODE_TX_RX, .parity = UART_PARITY_NONE, {.iqr = USART6_IRQn, .rx_pin_num = 55, .tx_pin_num = 54}},
};
static UART_T uart1;

static void hw_uart_init(com_port_t com);
static void hw_uart_gpio_init(com_port_t com, uint8_t priority);

/**
 * @brief       使能 串口时钟
 *
 * @param com   串口编号
 */
static inline void uart_clk_enable(com_port_t com)
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
        __HAL_RCC_USART3_CLK_ENABLE();
        break;

    default:
        break;
    }
}

/**
 * @brief           串口初始化
 *
 * @param com
 * @param priority
 */
void bsp_com_init(com_port_t com, uint8_t priority)
{

    hw_uart_gpio_init(com, priority);
    hw_uart_init(com);
}

/**
 * @brief           串口参数设置
 *
 * @param baud      波特率
 * @param parity    校验类型
 * @param mode      串口工作模式
 * @param rx_num    接收引脚
 * @param tx_num    发送引脚
 *
 * @note            在初始化前使用
 */
void bsp_set_uart_param(com_port_t com, uint32_t baud, uint32_t parity, uint32_t mode, uint8_t rx_num, uint8_t tx_num)
{
    uart_info_arr[com - 1].baund = baud;
    uart_info_arr[com - 1].parity = parity;
    uart_info_arr[com - 1].mode = mode;
    uart_info_arr[com - 1].gpio_info.rx_pin_num = rx_num;
    uart_info_arr[com - 1].gpio_info.tx_pin_num = tx_num;

    hw_uart_gpio_init(com, 5);
    hw_uart_init(com);
}

/**
 * @brief       初始硬件化串口
 *
 * @param com   com口编号
 */
static void hw_uart_init(com_port_t com)
{
    UART_HandleTypeDef uart_handle = {0};

    // 使能串口时钟
    uart_clk_enable(com);

    /*配置串口硬件参数 ######################################*/
    /* 异步串口模式 (UART Mode) */
    /* 配置如下:
      - 字长    = 8 位
      - 停止位  = 1 个停止位
      - 校验    = 无
      - 波特率  = 115200
      - 硬件流控制关闭 (RTS and CTS signals) */

    uart_handle.Instance = uart_info_arr[com - 1].instance;

    uart_handle.Init.BaudRate = uart_info_arr[com - 1].baund;
    uart_handle.Init.WordLength = UART_WORDLENGTH_8B;
    uart_handle.Init.StopBits = UART_STOPBITS_1;
    uart_handle.Init.Parity = uart_info_arr[com - 1].parity;
    uart_handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    uart_handle.Init.Mode = uart_info_arr[com - 1].mode;
    uart_handle.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&uart_handle) != HAL_OK)
    {
        _Error_Handler(__FILE__, __LINE__);
    }
}

/**
 * @brief       串口初始化
 *
 * @param com   串口编号 （com1~com6）
 * @param baud  波特率  （默认115200）
 */
static void hw_uart_gpio_init(com_port_t com, uint8_t priority)
{
    uint16_t rx_pin = 0, tx_pin = 0;
    GPIO_TypeDef *gpio = NULL;
    GPIO_InitTypeDef gpio_init = {};

    rx_pin = GET_PIN(uart_info_arr[com - 1].gpio_info.rx_pin_num);
    gpio = GET_PORT(uart_info_arr[com - 1].gpio_info.rx_pin_num);

    RCC->AHB1ENR |= (1 << rx_pin);

    /* 配置TX引脚 */
    gpio_init.Pin = tx_pin;
    gpio_init.Mode = GPIO_MODE_AF_PP;
    gpio_init.Pull = GPIO_PULLUP;
    gpio_init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio_init.Alternate = (com > 3 ? 0x08 : 0x07); // UART1~3 0x07
    HAL_GPIO_Init(gpio, &gpio_init);

    tx_pin = GET_PIN(uart_info_arr[com - 1].gpio_info.tx_pin_num);
    /* 配置RX引脚 */
    gpio_init.Pin = rx_pin;
    HAL_GPIO_Init(gpio, &gpio_init);

    /* 配置NVIC the NVIC for UART */
    HAL_NVIC_SetPriority(uart_info_arr[com - 1].gpio_info.iqr, priority, 0);
    HAL_NVIC_EnableIRQ(uart_info_arr[com - 1].gpio_info.iqr);

    CLEAR_BIT(uart_info_arr[com - 1].instance->SR, USART_SR_TC);   /* 清除TC发送完成标志 */
    CLEAR_BIT(uart_info_arr[com - 1].instance->SR, USART_SR_RXNE); /* 清除RXNE接收标志 */
    // USART_CR1_PEIE | USART_CR1_RXNEIE
    SET_BIT(uart_info_arr[com - 1].instance->CR1, USART_CR1_RXNEIE); /* 使能PE. RX接受中断 */
}


/**
 * @brief           填写数据到UART发送缓冲区,并启动发送中断
 *                  中断处理函数发送完毕后，自动关闭发送中断
 *
 * @param puart    串口指针
 * @param _usbuf   要发送的数据
 * @param _uslen    数据长度
 */
static void uartsend(UART_T *puart, uint8_t *_usbuf, uint16_t _uslen)
{
    uint16_t i;

    for (i = 0; i < _uslen; i++)
    {
        /* 如果发送缓冲区已经满了，则等待缓冲区空 */
        while (1)
        {
            __IO uint16_t usCount;

            DISABLE_INT();
            usCount = puart->ustxcount;
            ENABLE_INT();

            if (usCount < puart->ustxbufsize)
            {
                break;
            }
            else if (usCount == puart->ustxbufsize) /* 数据已填满缓冲区 */
            {
                if ((puart->uart->CR1 & USART_CR1_TXEIE) == 0)
                {
                    SET_BIT(puart->uart->CR1, USART_CR1_TXEIE);
                }
            }
        }

        /* 将新数据填入发送缓冲区 */
        puart->ptxbuf[puart->ustxwrite] = _usbuf[i];

        DISABLE_INT();
        if (++puart->ustxwrite >= puart->ustxbufsize)
        {
            puart->ustxwrite = 0;
        }
        puart->ustxcount++;
        ENABLE_INT();
    }

    SET_BIT(puart->uart->CR1, USART_CR1_TXEIE); /* 使能发送中断（缓冲区空） */
}
/*
*********************************************************************************************************
*	函 数 名: comSendBuf
*	功能说明: 向串口发送一组数据。数据放到发送缓冲区后立即返回，由中断服务程序在后台完成发送
*	形    参: _ucPort: 端口号(COM1 - COM8)
*			  _usbuf: 待发送的数据缓冲区
*			  _uslen : 数据长度
*	返 回 值: 无
*********************************************************************************************************
*/
void comsendbuf(com_port_t _ucPort, uint8_t *_usbuf, uint16_t _uslen)
{
    UART_T *puart;

    puart = comtouart(_ucPort);
    if (puart == 0)
    {
        return;
    }

    if (puart->sendbefor != 0)
    {
        puart->sendbefor(); /* 如果是RS485通信，可以在这个函数中将RS485设置为发送模式 */
    }
}
/*
*********************************************************************************************************
*	函 数 名: comSendChar
*	功能说明: 向串口发送1个字节。数据放到发送缓冲区后立即返回，由中断服务程序在后台完成发送
*	形    参: _ucPort: 端口号(COM1 - COM8)
*			  _ucByte: 待发送的数据
*	返 回 值: 无
*********************************************************************************************************
*/
void comsendchar(com_port_t _ucPort, uint8_t _ucByte)
{
    comsendbuf(_ucPort, &_ucByte, 1);
}

/*
 *
 *串口数据接收
 *
 */
/*
*********************************************************************************************************
*   函 数 名: UartTxEmpty
*   功能说明: 判断发送缓冲区是否为空。
*   形    参:  puart : 串口设备
*   返 回 值: 1为空。0为不空。
*********************************************************************************************************
*/
uint8_t uarttxempty(com_port_t _ucPort)
{
    UART_T *puart;
    uint8_t Sending;

    puart = comtouart(_ucPort);
    if (puart == 0)
    {
        return 0;
    }

    Sending = puart->sending;

    if (Sending != 0)
    {
        return 0;
    }
    return 1;
}
/*
*********************************************************************************************************
*	函 数 名: UartGetChar
*	功能说明: 从串口接收缓冲区读取1字节数据 （用于主程序调用）
*	形    参: puart : 串口设备
*			  _pByte : 存放读取数据的指针
*	返 回 值: 0 表示无数据  1表示读取到数据
*********************************************************************************************************
*/
static uint8_t uartgetchar(UART_T *puart, uint8_t *_pByte)
{
    uint16_t usCount;

    /* usRxWrite 变量在中断函数中被改写，主程序读取该变量时，必须进行临界区保护 */
    DISABLE_INT();
    usCount = puart->usrxcount;
    ENABLE_INT();

    /* 如果读和写索引相同，则返回0 */
    // if (puart->usRxRead == usRxWrite)
    if (usCount == 0) /* 已经没有数据 */
    {
        return 0;
    }
    else
    {
        *_pByte = puart->prxbuf[puart->usrxread]; /* 从串口接收FIFO取1个数据 */

        /* 改写FIFO读索引 */
        DISABLE_INT();
        if (++puart->usrxread >= puart->usRxbufsize)
        {
            puart->usrxread = 0;
        }
        puart->usrxcount--;
        ENABLE_INT();
        return 1;
    }
}

/*
*********************************************************************************************************
*	函 数 名: comgetchar
*	功能说明: 从接收缓冲区读取1字节，非阻塞。无论有无数据均立即返回。
*	形    参: _ucPort: 端口号(COM1 - COM8)
*			  _pByte: 接收到的数据存放在这个地址
*	返 回 值: 0 表示无数据, 1 表示读取到有效字节
*********************************************************************************************************
*/
uint8_t comgetchar(com_port_t _ucPort, uint8_t *_pByte)
{
    UART_T *puart;

    puart = comtouart(_ucPort);
    if (puart == 0)
    {
        return 0;
    }

    return uartgetchar(puart, _pByte);
}
/*
*********************************************************************************************************
*	函 数 名: UartIRQ
*	功能说明: 供中断服务程序调用，通用串口中断处理函数
*	形    参: puart : 串口设备
*	返 回 值: 无
*********************************************************************************************************
*/
static void UartIRQ(UART_T *puart)
{
    uint32_t isrflags = READ_REG(puart->uart->SR);
    uint32_t cr1its = READ_REG(puart->uart->CR1);
    uint32_t cr3its = READ_REG(puart->uart->CR3);

    /* 处理接收中断  */
    if ((isrflags & USART_SR_RXNE) != RESET)
    {
        /* 从串口接收数据寄存器读取数据存放到接收FIFO */
        uint8_t ch;

        ch = READ_REG(puart->uart->DR);                /* 读取串口接收寄存器 */
        puart->prxbuf[puart->usrxwrite] = ch;         /* 填入串口接收FIFO */
        if (++puart->usrxwrite >= puart->usRxbufsize) /* 接收 FIFO 的写指针+1 */
        {
            puart->usrxwrite = 0;
        }
        if (puart->usrxcount < puart->usRxbufsize) /* 统计未处理的字节个数 */
        {
            puart->usrxcount++;
        }

        /* 回调函数,通知应用程序收到新数据,一般是发送1个消息或者设置一个标记 */
        {
            if (puart->recivenew)
            {
                puart->recivenew(ch); /* 比如，交给MODBUS解码程序处理字节流 */
            }
        }
    }

    /* 处理发送缓冲区空中断 */
    if (((isrflags & USART_SR_TXE) != RESET) && (cr1its & USART_CR1_TXEIE) != RESET)
    {
        if (puart->ustxcount == 0)
        {
            /* 发送缓冲区的数据已取完时， 禁止发送缓冲区空中断 （注意：此时最后1个数据还未真正发送完毕）*/
            // USART_ITConfig(puart->uart, USART_IT_TXE, DISABLE);
            CLEAR_BIT(puart->uart->CR1, USART_CR1_TXEIE);

            /* 使能数据发送完毕中断 */
            // USART_ITConfig(puart->uart, USART_IT_TC, ENABLE);
            SET_BIT(puart->uart->CR1, USART_CR1_TCIE);
        }
        else
        {
            puart->sending = 1;

            /* 从发送FIFO取1个字节写入串口发送数据寄存器 */
            // USART_SendData(puart->uart, puart->pTxBuf[puart->usTxRead]);
            puart->uart->DR = puart->ptxbuf[puart->ustxread];
            if (++puart->ustxread >= puart->ustxbufsize)
            {
                puart->ustxread = 0;
            }
            puart->ustxcount--;
        }
    }
    /* 数据bit位全部发送完毕的中断 */
    if (((isrflags & USART_SR_TC) != RESET) && ((cr1its & USART_CR1_TCIE) != RESET))
    {
        // if (puart->usTxRead == puart->usTxWrite)
        if (puart->ustxcount == 0)
        {
            /* 如果发送FIFO的数据全部发送完毕，禁止数据发送完毕中断 */
            // USART_ITConfig(puart->uart, USART_IT_TC, DISABLE);
            CLEAR_BIT(puart->uart->CR1, USART_CR1_TCIE);

            /* 回调函数, 一般用来处理RS485通信，将RS485芯片设置为接收模式，避免抢占总线 */
            if (puart->sendover)
            {
                puart->sendover();
            }

            puart->sending = 0;
        }
        else
        {
            /* 正常情况下，不会进入此分支 */

            /* 如果发送FIFO的数据还未完毕，则从发送FIFO取1个数据写入发送数据寄存器 */
            // USART_SendData(puart->uart, puart->pTxBuf[puart->usTxRead]);
            puart->uart->DR = puart->ptxbuf[puart->ustxread];
            if (++puart->ustxread >= puart->ustxbufsize)
            {
                puart->ustxread = 0;
            }
            puart->ustxcount--;
        }
    }
}

//===============================COM 版本==============================

static char *itoa(int value, char *string, int radix);

static void uart_x_8bit(UART_HandleTypeDef *huart, uint8_t ch)
{
    huart->Instance->DR = ch;
}

void uart_init(void)
{
    // 初始化ringbuffer
    ringbuffer_init(&rx_rb, rx_buffer, UART_RX_BUF_SIZE);

    uart1.info.huart = &huart1;
    uart1.info.status = UART_IDEL;

    uart1.rx_Count = 0;
    uart1.rx_Buffer = &rx_rb;

    // 开启 接受中断
    HAL_UART_Receive_IT(&huart1, (uint8_t *)uart_rx_temp, REC_LENGTH);
}

void UARTx_SendData(UART_HandleTypeDef *huart, uint8_t *_usbuf, uint16_t length)
{
    HAL_UART_Transmit(huart, _usbuf, length, 0xffff);
}

/*
 * 函数名：USART_printf
 * 描述  ：格式化输出，类似于C库中的printf，但这里没有用到C库
 * 输入  ：-USARTx 串口通道，这里只用到了串口2，即USART2
 *		     -Data   要发送到串口的内容的指针
 *			   -...    其他参数
 * 输出  ：无
 * 返回  ：无
 * 调用  ：外部调用
 *         典型应用USART2_printf( USART2, "\r\n this is a demo \r\n" );
 *            		 USART2_printf( USART2, "\r\n %d \r\n", i );
 *            		 USART2_printf( USART2, "\r\n %s \r\n", j );
 */
static uint8_t buffer[2] = {0x0d, 0x0a};
void UARTx_printf(UART_HandleTypeDef *huart, char *Data, ...)
{
    const char *s;
    int d;
    char _usbuf[16];

    va_list ap;
    va_start(ap, Data);

    while (*Data != 0) // 判断是否到达字符串结束符
    {
        if (*Data == 0x5c) //'\'
        {
            switch (*++Data)
            {
            case 'r': // 回车符
                uart_x_8bit(huart, buffer[0]);
                Data++;
                break;

            case 'n': // 换行符
                uart_x_8bit(huart, buffer[1]);
                Data++;
                break;

            default:
                Data++;
                break;
            }
        }

        else if (*Data == '%')
        { //
            switch (*++Data)
            {
            case 's': // 字符串
                s = va_arg(ap, const char *);

                for (; *s; s++)
                {
                    uart_x_8bit(huart, *s);
                    while (!(READ_BIT(huart->Instance->SR, USART_SR_TXE) == (USART_SR_TXE)))
                        ;
                }

                Data++;

                break;

            case 'd':
                // 十进制
                d = va_arg(ap, int);

                itoa(d, _usbuf, 10);

                for (s = _usbuf; *s; s++)
                {
                    uart_x_8bit(huart, *s);
                    while ((READ_BIT(huart->Instance->SR, USART_SR_TXE) == (USART_SR_TXE)) == 0)
                        ;
                }

                Data++;

                break;

            default:
                Data++;

                break;
            }
        }

        else
        {
            uart_x_8bit(huart, *Data);
            Data++;
        }
        while ((READ_BIT(huart->Instance->SR, USART_SR_TXE) == (USART_SR_TXE)) == 0)
            ;
    }
}

/**
 * @brief 因地制宜 不一定要在这个函数中实现
 *  根据自己需求定时将数据提取到 目的数组中
 * 放在定时器中 50ms 一次(按需修改 )
 *
 */
__weak void UART_ProcessData(void)
{
    uint16_t len = ringbuffer_data_len(uart1.rx_Buffer);

    if (!len)
        return;

    if (uart1.info.status != UART_REVICING)
        return;
    uint8_t temp = 0;

    for (int i = 0; i < len; i++)
    {
        ringbuffer_getchar(uart1.rx_Buffer, &temp);
        // _usbuf[i] = temp;
    }

    /* 这里在提取完毕数据后清空
        if (一帧结束判定条件)
            uart1.rx_Count = 0;
            uart1.info.status = UART_REVICED;
    */
}

/**
 * @brief 串口中断回调函数
 * @param 调用回调函数的串口
 * @note  串口每次收到数据以后都会关闭中断，如需重复使用，必须再次开启
 * @retval None
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        ringbuffer_putchar(&rx_rb, uart_rx_temp[0]);
        uart1.rx_Count++;
        HAL_UART_Receive_IT(&huart1, uart_rx_temp, REC_LENGTH);
        if (uart1.info.status == UART_IDEL)
        {
            uart1.info.status = UART_REVICING;
        }
    }
}

/*
 * 函数名：itoa
 * 描述  ：将整形数据转换成字符串
 * 输入  ：-radix =10 表示10进制，其他结果为0
 *         -value 要转换的整形数
 *         -_usbuf 转换后的字符串
 *         -radix = 10
 * 输出  ：无
 * 返回  ：无
 */
static char *itoa(int value, char *string, int radix)
{
    int i, d;
    int flag = 0;
    char *ptr = string;

    /* This implementation only works for decimal numbers. */
    if (radix != 10)
    {
        *ptr = 0;
        return string;
    }

    if (!value)
    {
        *ptr++ = 0x30;
        *ptr = 0;
        return string;
    }

    /* if this is a negative value insert the minus sign. */
    if (value < 0)
    {
        *ptr++ = '-';

        /* Make the value positive. */
        value *= -1;
    }

    for (i = 10000; i > 0; i /= 10)
    {
        d = value / i;

        if (d || flag)
        {
            *ptr++ = (char)(d + 0x30);
            value -= (d * i);
            flag = 1;
        }
    }

    /* Null terminate the string. */
    *ptr = 0;

    return string;

} /* NCL_Itoa */

// int fputc(int ch, FILE *f)
//{
// #if 1 /* 将需要printf的字符通过串口中断FIFO发送出去，printf函数会立即返回 */
//     comsendchar(COM1, ch);

//    return ch;
// #else /* 采用阻塞方式发送每个字符,等待数据发送完毕 */
//    /* 写一个字节到USART1 */
//    USART1->DR = ch;

//    /* 等待发送结束 */
//    while ((USART1->SR & USART_SR_TC) == 0)
//    {
//    }

//    return ch;
// #endif
//}

// int fgetc(FILE *f)
//{

// #if 1 /* 从串口接收FIFO中取1个数据, 只有取到数据才返回 */
//     uint8_t ucData;

//    while (comgetchar(COM1, &ucData) == 0)
//        ;

//    return ucData;
// #else
//    /* 等待接收到数据 */
//    while ((USART1->SR & USART_SR_RXNE) == 0)
//    {
//    }

//    return (int)USART1->DR;
// #endif
//}

////============================= END OF FILE ===================================
