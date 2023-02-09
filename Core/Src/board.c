/**
 * @file board.c
 * @author BlackSheep (blacksheep.208h@gmail.com)
 * @brief   板级初始化
 * @version 0.1
 * @date 2023-02-06
 *
 * @copyright Copyright (c) 2023
 *
 */

#include <stdio.h>
#include "board.h"

void board_init(void)
{
    uint8_t error = 1;

    delay_Init();
    Led_Init();
    DHT11_Init();
    printf("board peripherals are initialized.\r\n");
}

// 寄存器版本
int fputc(int ch, FILE *f)
{
    // 具体哪个串口可以更改USART1为其它串口
    while ((USART1->SR & 0X40) == 0)
        ; // 循环发送,直到发送完毕
    USART1->DR = (uint8_t)ch;
    return ch;
}
