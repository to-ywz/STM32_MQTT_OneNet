/**
 * @file dht11_ops.c
 * @author BlackSheep (blacksheep.208h@gmail.com)
 * @brief   1.实现DHT11温湿度传感器的通讯
 *          2. 单总线通信
 *          2.湿度整数+湿度小数+温度整数+温度小数+校验和
 * @version 0.1
 * @date 2023-02-09
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "stddef.h"
#include "dht11_ops.h"
#include "systick.h"

/*从DHT11读取一个位,返回值：1/0*/
static uint8_t dht11_read_bit(dht11_object_t *dht);
/*从DHT11读取一个字节,返回值：读到的数据*/
static uint8_t dht11_read_byte(dht11_object_t *dht);
/*复位DHT11*/
static void dht11_reset(dht11_object_t *dht);
/*等待DHT11的回应，返回1：未检测到DHT11的存在；返回0：存在*/
static dht11_error_et dht11_get_status(dht11_object_t *dht);

/*从DHT11,temp:温度值(0-50),humi:湿度值(20%-90%),返回值：0,正常;1,失败*/

/**
 * @brief       读取数据
 *
 * @param dht   dht11对象指针
 * @retval      DHT11_NOERROR   :成功
 *              DHT11_DATAERROR :数据异常
 */
dht11_error_et dht11_value_read(dht11_object_t *dht)
{
    dht11_error_et error = DHT11_NONE;
    uint8_t readBuffer[5];

    dht11_reset(dht);
    if (dht11_get_status(dht) == DHT11_NOERROR)
    {
        for (int i = 0; i < 5; i++)
        {
            readBuffer[i] = dht11_read_byte(dht);
        }

        uint8_t checkSum = 0;
        checkSum = readBuffer[0] + readBuffer[1] + readBuffer[2] + readBuffer[3];
        error = DHT11_DATAERROR;
        if (checkSum == readBuffer[4])
        {
            dht->temperature = (float)(readBuffer[2] * 100 + readBuffer[3]) / 100;
            dht->humidity = (float)(readBuffer[0] * 100 + readBuffer[1]) / 100;
            error = DHT11_NOERROR;
        }
    }
    return error;
}

/**
 * @brief               dht11 指针
 *
 * @param dht           需要初始化对象
 * @param setPinStatus  设置总线输出值
 * @param getPinStatus  读取总线输入值
 * @param mode          配置总线的输入输出模式
 * @param delayms       毫秒延时
 * @param delayus       微秒延时
 * @retval              DHT11_INITERROR :初始化错误
 *                      DHT11_NONE      :DHT11不存在
 *                      DHT11_NOERROR   :成功
 */
dht11_error_et dht11_ops_init(dht11_object_t *dht,
                              dht11_pinwrite_t setPinStatus,
                              dht11_pinread_t getPinStatus,
                              dht11_setbusmode_t mode,
                              dht11_delay_t delayms,
                              dht11_delay_t delayus)
{
    if ((dht == NULL) || (setPinStatus == NULL) || (getPinStatus == NULL) || (mode == NULL) || (delayms == NULL) || (delayus == NULL))
    {
        return DHT11_INITERROR;
    }
    dht->set_Pin_value = setPinStatus;
    dht->read_pin_value = getPinStatus;
    dht->set_pin_mode = mode;
    dht->delayms = delayms;
    dht->delayus = delayus;

    dht->humidity = 0.0;
    dht->temperature = 0.0;

    dht11_reset(dht);
    return dht11_get_status(dht);
}

/**
 * @brief       复位DHT11，开始通讯
 *
 * @param dht   dht11对象指针
 */
static void dht11_reset(dht11_object_t *dht)
{
    dht->set_pin_mode(DHT11_OUT);    // 设置为输出方式
    dht->set_Pin_value(DHT11_RESET); // 将引脚点位拉低
    dht->delayms(20);                // 拉低至少18ms
    dht->set_Pin_value(DHT11_SET);   // 拉高
    dht->delayus(30);                // 主机拉高20至40us
}

/**/

/**
 * @brief       等待DHT11的回应
 * @param dht   DHT11对象指针
 * @retval      DHT11_NONE      :不存在
 *              DHT11_NOERROR   :存在
 */
static dht11_error_et dht11_get_status(dht11_object_t *dht)
{
    uint8_t retry = 0;
    dht->set_pin_mode(DHT11_IN); // 设置为输入方式
    while (dht->read_pin_value() && (retry < 100))
    {
        retry++;
        dht->delayus(1);
    }
    if (retry >= 100)
    {
        return DHT11_NONE;
    }
    retry = 0;
    while (!dht->read_pin_value() && (retry < 100))
    {
        retry++;
        dht->delayus(1);
    }
    if (retry >= 100)
    {
        return DHT11_NONE;
    }
    return DHT11_NOERROR;
}

/**
 * @brief       读取一个位
 *
 * @param dht   dht11 指针
 * @retval      当前位电平
 */
static uint8_t dht11_read_bit(dht11_object_t *dht)
{
    uint8_t retry = 0;
    /*等待变为低电平*/
    while (dht->read_pin_value() && (retry < 100))
    {
        retry++;
        dht->delayus(1);
    }
    retry = 0;
    /*等待变高电平*/
    while (!dht->read_pin_value() && (retry < 100))
    {
        retry++;
        dht->delayus(1);
    }
    dht->delayus(40); // 延时判断此位是0还是1

    return dht->read_pin_value();
}

/**
 * @brief       读取一字节
 *
 * @param dht   dht11对象指针
 * @retval      读到的数据
 */
static uint8_t dht11_read_byte(dht11_object_t *dht)
{
    uint8_t data = 0;
    for (int i = 0; i < 8; i++)
    {
        data <<= 1;
        data |= dht11_read_bit(dht);
    }

    return data;
}
//==================================================End=================================================
