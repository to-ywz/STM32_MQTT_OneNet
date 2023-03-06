/**
 * @file dht11.c
 * @author BlackSheep (blacksheep.208h@gmail.com)
 * @brief   DHT11 物理层对接
 * @version 0.1
 * @date 2023-02-09
 *
 * @copyright Copyright (c) 2023
 *
 */

#include <stdio.h>

#include "dht11_ops.h"
#include "dht11.h"
#include "bsp_gpio.h"
#include "systick.h"
#include "sys.h"

#define DHT11_DATA_PIN get_pinnum("PE3")

typedef struct DHT11
{
    uint8_t num;        // 编号
    dht11_object_t obj; // DHT11 基本操作

} dht11_t;

static dht11_t g_dht11;

/*定义设置DHT11引脚的输出值函数类型*/
static void dht11_pin_write(dht11_busvalue_et setValue)
{
    bsp_pin_write(DHT11_DATA_PIN, setValue);
}

/*定义读取引脚电平函数类型*/
static uint8_t dht11_pin_read(void)
{
    return bsp_pin_read(DHT11_DATA_PIN);
}

/*定义设置引脚的输入输出方向函数类型*/
static void dht11_pin_mode(dht11_busmode_et mode)
{
    if (DHT11_OUT == mode)
        bsp_pin_mode(DHT11_DATA_PIN, PIN_MODE_OUTPUT);
    else
        bsp_pin_mode(DHT11_DATA_PIN, PIN_MODE_INPUT);
}

/**
 * @brief DHT11 获取数据
 *
 */
void dht11_data_update(uint8_t id)
{
    // float temperature; // 温度值
    // float humidity;    // 湿度值

    dht11_value_read(&g_dht11.obj);

    // temperature = g_dht11.obj.temperature;
    // humidity = g_dht11.obj.humidity;
}

/**
 * @brief DHT11 初始化
 *
 */
void dht11_init(void)
{
    dht11_error_et error = DHT11_NOERROR;

    g_dht11.num = 0;

    error = dht11_ops_init(&g_dht11.obj,
                           dht11_pin_write,
                           dht11_pin_read,
                           dht11_pin_mode,
                           delayms,
                           delayus);

    if (DHT11_NOERROR == error)
    {
        printf("DHT11 is initialized.\r\n");
    }
    else if (DHT11_INITERROR == error)
    {
        printf("Initialize function error.\r\n");
    }
    else if (DHT11_NONE == error)
    {
        printf("Can't find DHT11 device.\r\n");
    }
    dht11_value_read(&g_dht11.obj); // 预先读取一次,这次数据可能是错的
}

/*===============================对外API=================================*/
/**
 * @brief   获取湿度数据
 *
 * @param id dht11编号
 * @retval  湿度
 */
inline float dht11_get_humidity(uint8_t id)
{
    return g_dht11.obj.humidity;
}

/**
 * @brief   获取温度数据
 *
 * @param id dht11编号
 * @retval  温度
 */
inline float dht11_get_temperature(uint8_t id)
{
    return g_dht11.obj.temperature;
}

//==================================================End=================================================
