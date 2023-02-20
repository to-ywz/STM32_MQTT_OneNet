/**
 * @file hw_dht11.c
 * @author BlackSheep (blacksheep.208h@gmail.com)
 * @brief   DHT11 物理层对接
 * @version 0.1
 * @date 2023-02-09
 *
 * @copyright Copyright (c) 2023
 *
 */

#include <stdio.h>

#include "dht11.h"
#include "hw_dht11.h"
#include "bsp_gpio.h"
#include "delay.h"

#define DHT11_DATA_PIN 20

Dht11_t G_dht11;

/*定义设置DHT11引脚的输出值函数类型*/
static void Dht11_pin_write(Dht11_BusValue_t setValue)
{
    bsp_pin_write(DHT11_DATA_PIN, setValue);
}

/*定义读取引脚电平函数类型*/
static uint8_t Dht11_pin_read(void)
{
    return bsp_pin_read(DHT11_DATA_PIN);
}

/*定义设置引脚的输入输出方向函数类型*/
static void Dht11_pin_mode(Dht11_BusMode_t mode)
{
    if (DHT11_Out == mode)
        bsp_pin_mode(DHT11_DATA_PIN, PIN_MODE_OUTPUT);
    else
        bsp_pin_mode(DHT11_DATA_PIN, PIN_MODE_INPUT);
}

/**
 * @brief DHT11 获取数据
 *
 */
static void DHT11_get_data(void)
{
    float temperature; // 温度值
    float humidity;    // 湿度值

    GetProcessValueFromDHT11(&G_dht11.obj);

    temperature = G_dht11.obj.temperature;
    humidity = G_dht11.obj.humidity;
}

/**
 * @brief DHT11 初始化
 *
 */
void DHT11_Init(void)
{
    Dht11_Error_t error = DHT11_NoError;

    G_dht11.num = 0;
    G_dht11.dataUpdate = DHT11_get_data;

    error = InitializeDHT11(&G_dht11.obj,
                            Dht11_pin_write,
                            Dht11_pin_read,
                            Dht11_pin_mode,
                            delay_ms,
                            delay_us);

    if (DHT11_NoError == error)
    {
        printf("DHT11 is initialized.\r\n");
    }
    else if (DHT11_InitError == error)
    {
        printf("Initialize function error.\r\n");
    }
    else if (DHT11_None == error)
    {
        printf("Can't find DHT11 device.\r\n");
    }
    G_dht11.dataUpdate();
}
//==================================================End=================================================
