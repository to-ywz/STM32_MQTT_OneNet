/**
 * @file dht11.h
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
#include "dht11.h"

/*从DHT11读取一个位,返回值：1/0*/
static uint8_t Dht11_read_bit(Dht11_Object_t *dht);
/*从DHT11读取一个字节,返回值：读到的数据*/
static uint8_t Dht11_read_byte(Dht11_Object_t *dht);
/*复位DHT11*/
static void Dht11_reset(Dht11_Object_t *dht);
/*等待DHT11的回应，返回1：未检测到DHT11的存在；返回0：存在*/
static Dht11_Error_t Dht11_get_status(Dht11_Object_t *dht);

/*从DHT11读取数据,temp:温度值(0-50),humi:湿度值(20%-90%),返回值：0,正常;1,失败*/
Dht11_Error_t GetProcessValueFromDHT11(Dht11_Object_t *dht)
{
    Dht11_Error_t error = DHT11_None;
    uint8_t readBuffer[5];

    Dht11_reset(dht);
    if (Dht11_get_status(dht) == DHT11_NoError)
    {
        for (int i = 0; i < 5; i++)
        {
            readBuffer[i] = Dht11_read_byte(dht);
        }

        uint8_t checkSum = 0;
        checkSum = readBuffer[0] + readBuffer[1] + readBuffer[2] + readBuffer[3];
        error = DHT11_DataError;
        if (checkSum == readBuffer[4])
        {
            dht->temperature = (float)(readBuffer[2] * 100 + readBuffer[3]) / 100;
            dht->humidity = (float)(readBuffer[0] * 100 + readBuffer[1]) / 100;
            error = DHT11_NoError;
        }
    }
    return error;
}

/*DHT11初始化操作*/
Dht11_Error_t InitializeDHT11(Dht11_Object_t *dht,           // 需要初始化对象
                              Dht11_PinWrite_t setPinStatus, // 设置总线输出值
                              Dht11_PinRead_t getPinStatus,  // 读取总线输入值
                              Dht11_SetBusMode_t mode,       // 配置总线的输入输出模式
                              Dht11_Delay_t delayms,         // 毫秒延时
                              Dht11_Delay_t delayus          // 微秒延时
)
{
    if ((dht == NULL) || (setPinStatus == NULL) || (getPinStatus == NULL) || (mode == NULL) || (delayms == NULL) || (delayus == NULL))
    {
        return DHT11_InitError;
    }
    dht->set_Pin_value = setPinStatus;
    dht->read_pin_value = getPinStatus;
    dht->set_pin_mode = mode;
    dht->Delayms = delayms;
    dht->Delayus = delayus;

    dht->humidity = 0.0;
    dht->temperature = 0.0;

    Dht11_reset(dht);
    return Dht11_get_status(dht);
}

/*复位DHT11，开始通讯*/
static void Dht11_reset(Dht11_Object_t *dht)
{
    dht->set_pin_mode(DHT11_Out);    // 设置为输出方式
    dht->set_Pin_value(DHT11_Reset); // 将引脚点位拉低
    dht->Delayms(20);                // 拉低至少18ms
    dht->set_Pin_value(DHT11_Set);   // 拉高
    dht->Delayus(30);                // 主机拉高20至40us
}

/*等待DHT11的回应，返回1：未检测到DHT11的存在；返回0：存在*/
static Dht11_Error_t Dht11_get_status(Dht11_Object_t *dht)
{
    uint8_t retry = 0;
    dht->set_pin_mode(DHT11_In); // 设置为输入方式
    while (dht->read_pin_value() && (retry < 100))
    {
        retry++;
        dht->Delayus(1);
    }
    if (retry >= 100)
    {
        return DHT11_None;
    }
    retry = 0;
    while (!dht->read_pin_value() && (retry < 100))
    {
        retry++;
        dht->Delayus(1);
    }
    if (retry >= 100)
    {
        return DHT11_None;
    }
    return DHT11_NoError;
}

/*从DHT11读取一个位,返回值：1/0*/
static uint8_t Dht11_read_bit(Dht11_Object_t *dht)
{
    uint8_t retry = 0;
    /*等待变为低电平*/
    while (dht->read_pin_value() && (retry < 100))
    {
        retry++;
        dht->Delayus(1);
    }
    retry = 0;
    /*等待变高电平*/
    while (!dht->read_pin_value() && (retry < 100))
    {
        retry++;
        dht->Delayus(1);
    }
    dht->Delayus(40); // 延时判断此位是0还是1

    return dht->read_pin_value();
}

/*从DHT11读取一个字节,返回值：读到的数据*/
static uint8_t Dht11_read_byte(Dht11_Object_t *dht)
{
    uint8_t data = 0;
    for (int i = 0; i < 8; i++)
    {
        data <<= 1;
        data |= Dht11_read_bit(dht);
    }

    return data;
}
