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
#include <string.h>
#include "board.h"

static sf_timer_t sftimer[2];

static void timer_task_init(void);

void board_init(void)
{
    systick_init();
    Led_Init();
    timer_task_init();
    bsp_com_init(COM1);
    // bsp_uart_param_setup(USART1, 115200, UART_PARITY_NONE, UART_MODE_TX_RX);
    com_databuf_send(COM1, (uint8_t *)"ABCDEF.\r\n", 11);
    printf("board peripherals are initialized. by UART1\r\n");
}

static void timer1_task(void)
{
    Led_CheckMode();
}

static void timer_task_init(void)
{
    sf_timer_init(&sftimer[0], timer1_task, 50, 50);
    sf_timer_start(&sftimer[0]);
}

/**
 * @brief       获得引脚编号
 *
 * @param str   类似PA10类型的字符
 * @retval      引脚编号(255为非法)
 */
uint8_t get_pinnum(char *str)
{
    // 将字符转为小写
    str[0] |= 0x20, str[1] |= 0x20;
    if (str[0] != 'p')
        return 255;

    int len = strlen(str);
    uint8_t pin = 0, port = 0;

    if (len != 3 && len != 4)
        return 255;

    port = str[1] - 'a';
    pin = str[2] - '0';
    if (len > 3)
        pin = pin * 10 + str[3] - '0';

    return (port * 16 + pin);
}
/**
 * @brief 系统复位函数
 *
 */
void board_system_reset(void)
{
    __set_FAULTMASK(1); // 关闭所有中断
    NVIC_SystemReset(); // 复位
}

//==================================================End=================================================
