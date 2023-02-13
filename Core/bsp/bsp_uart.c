
#include <string.h>
#include "bsp_uart.h"
#include "usart.h"

#include <stdarg.h>
//------------------------------ 用户宏定义 ----------------------------------
//------------------------------ 用户变量定义 --------------------------------
uint8_t pRxBuffer[USART_RX_BUF_SIZE];
uint8_t pTxBuffer[USART_TX_BUF_SIZE];
uint8_t UARTx_temp[REC_LENGTH];
uint8_t rxCount;
uint8_t txCount;

static struct ringbuffer rx_rb;
UartObject_t uart2;

static char *itoa(int value, char *string, int radix);

static void USARTx_TransmitData8(UART_HandleTypeDef *huart, uint8_t ch)
{
    huart->Instance->DR = ch;
}

static UART_Tranmist(char *data);

void uart_init(void)
{
    // 初始化ringbuffer
    ringbuffer_init(&rx_rb, pRxBuffer, USART_RX_BUF_SIZE);

    uart2.info.huart = &huart2;
    uart2.info.status = UART_IDEL;

    uart2.rx_Count = 0;
    uart2.rx_Buffer = &rx_rb;
}

static UART_TranmistByte(uint8_t byte);

void uart_sendByte(UART_HandleTypeDef *huart, uint8_t ch)
{
#if USING_RS485
    RS485_TX_EN();
    delayus(5);
#endif

    HAL_UART_Transmit(huart, &ch, 1, 1000);

#if USING_RS485
    RS485_RX_EN();
    delayus(5);
#endif
}

/**
 * @brief Construct a new rx Data Handler object
 * @param None
 * @retval None
 */
uint8_t UARTFlag;
char dataString[10][10];
void rxDataHandler(void)
{
    if (0 == rxCount)
    {
        UARTFlag = 0;
        printf("未接受到数据\r\n");
        return;
    }

    // 初始化 数据 缓存
    uint8_t i = 0;
    char buf[64] = {0};
    strcpy(buf, (char *)pRxBuffer);
    memset(dataString, 0, sizeof(dataString));

    char *pString = strtok((char *)(buf), "\n");
    while (pString)
    {
        strcpy(dataString[i++], pString);
        printf("%s\r\n", pString);
        pString = strtok(NULL, "\n");
    }
    rxCount = 0;
    memset(pRxBuffer, 0, sizeof(pRxBuffer));
    UARTFlag = 1;
}

uint8_t Board_UARTGetCharBlocking(void)
{
#if USING_RS485
    RS485_RX_EN();
#endif
    uint8_t ch;
    while (0 == ringbuffer_data_len(&rx_rb))
        ;

    ringbuffer_getchar(&rx_rb, &ch);
    // printf("%x\r\n", ch);
    return ch;
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
        ringbuffer_putchar(&rx_rb, UARTx_temp[0]);
        HAL_UART_Receive_IT(&huart2, (uint8_t *)UARTx_temp, REC_LENGTH);
    }
}

void uart_sendString(UART_HandleTypeDef *huart, char *str)
{
    HAL_UART_Transmit(huart, (uint8_t *)str, strlen(str), 0xfff);
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
uint8_t buffer[2] = {0x0d, 0x0a};
void USART_printf(UART_HandleTypeDef *huart, char *Data, ...)
{
    const char *s;
    int d;
    char buf[16];

    va_list ap;
    va_start(ap, Data);

    while (*Data != 0) // 判断是否到达字符串结束符
    {
        if (*Data == 0x5c) //'\'
        {
            switch (*++Data)
            {
            case 'r': // 回车符
                USARTx_TransmitData8(huart, buffer[0]);
                Data++;
                break;

            case 'n': // 换行符
                USARTx_TransmitData8(huart, buffer[1]);
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
                    USARTx_TransmitData8(huart, *s);
                    while (!(READ_BIT(huart->Instance->SR, USART_SR_TXE) == (USART_SR_TXE)))
                        ;
                }

                Data++;

                break;

            case 'd':
                // 十进制
                d = va_arg(ap, int);

                itoa(d, buf, 10);

                for (s = buf; *s; s++)
                {
                    USARTx_TransmitData8(huart, *s);
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
            USARTx_TransmitData8(huart, *Data);
            Data++;
        }
        while ((READ_BIT(huart->Instance->SR, USART_SR_TXE) == (USART_SR_TXE)) == 0)
            ;
    }
}

/*
 * 函数名：itoa
 * 描述  ：将整形数据转换成字符串
 * 输入  ：-radix =10 表示10进制，其他结果为0
 *         -value 要转换的整形数
 *         -buf 转换后的字符串
 *         -radix = 10
 * 输出  ：无
 * 返回  ：无
 * 调用  ：被USART2_printf()调用
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

//============================= END OF FILE ===================================
