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

static sf_timer_t sftimer[2];

static void timer_task_init(void);

void board_init(void)
{
    systick_init();
    Led_Init();
    timer_task_init();
    bsp_com_init(COM1);
    bsp_uart_param_setup(USART1, 115200, UART_PARITY_NONE, UART_MODE_TX_RX);
    com_buf_send(COM1, (uint8_t *)"ABCDEF.\r\n", 11);
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
 * @brief 系统复位函数
 *
 */
void board_system_reset(void)
{
    __set_FAULTMASK(1); // 关闭所有中断
    NVIC_SystemReset(); // 复位
}

//==================================================End=================================================
