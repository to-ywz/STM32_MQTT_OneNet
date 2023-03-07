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
#include "spi.h"
#include "dht11.h"

static sf_timer_t sftimer[2];

static void timer_task_init(void);

void board_init(void)
{
    uint8_t rx_buf = 0xAA, tx_buf = 0;
    systick_init();
    timer_task_init();
    bsp_com_init(COM1);

    Led_Init();
    dht11_init();
    nrf24l01_init();

    printf("board peripherals are initialized. by UART1\r\n");
}

static void timer1_task(void)
{
    Led_CheckMode();
}

static void timer2_task(void)
{
    dht11_data_update(0);
    printf("humidity = %.2f.\t", dht11_get_humidity(0));
    nrf24l01_data_xmit(dht11_get_humidity(0));
    //nrf24l01_data_recv();
}

static void timer_task_init(void)
{
    sf_timer_init(&sftimer[0], timer1_task, 50, 50);
    sf_timer_init(&sftimer[1], timer2_task, 1000, 1000);
    sf_timer_start(&sftimer[0]);
    sf_timer_start(&sftimer[1]);
}

//==================================================End=================================================
