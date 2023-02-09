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
#ifndef __DHT11FUNCTION_H
#define __DHT11FUNCTION_H

#include "stdint.h"

/*定义DHT11错误消息枚举*/
typedef enum DHT11Error
{
    DHT11_NoError,   // 没有错误
    DHT11_None,      // 未检测到DHT11
    DHT11_InitError, // 初始化错误
    DHT11_DataError  // 通讯校验错误
} Dht11_Error_t;

/*定义单总线方向枚举*/
typedef enum DHT11IOMode
{
    DHT11_In = 0,
    DHT11_Out = 1
} Dht11_BusMode_t;

/*定义单总线操作值枚举*/
typedef enum DhtPinValue
{
    DHT11_Reset = 0,
    DHT11_Set = 1
} Dht11_BusValue_t;

/* 定义DHT11对象类型 */
typedef struct Dht11Object
{
    float temperature; // 温度值
    float humidity;    // 湿度值

    void (*set_Pin_value)(Dht11_BusValue_t setValue); // 设置DHT11引脚的输出值
    uint8_t (*read_pin_value)(void);                  // 读取引脚电平
    void (*set_pin_mode)(Dht11_BusMode_t mode);       // 设置引脚的输入输出方向

    void (*Delayms)(volatile uint32_t nTime); // 实现ms延时操作
    void (*Delayus)(volatile uint32_t nTime); // 实现us延时操作
} Dht11_Object_t;

/*定义设置DHT11引脚的输出值函数类型*/
typedef void (*Dht11_PinWrite_t)(Dht11_BusValue_t setValue);
/*定义读取引脚电平函数类型*/
typedef uint8_t (*Dht11_PinRead_t)(void);
/*定义设置引脚的输入输出方向函数类型*/
typedef void (*Dht11_SetBusMode_t)(Dht11_BusMode_t mode);
/*定义实现ms延时操作函数类型*/
typedef void (*Dht11_Delay_t)(volatile uint32_t nTime);

/*从DHT11读取数据,temp:温度值(0-50),humi:湿度值(20%-90%),返回值：0,正常;1,失败*/
Dht11_Error_t GetProcessValueFromDHT11(Dht11_Object_t *dht);

/*DHT11初始化操作*/
Dht11_Error_t InitializeDHT11(Dht11_Object_t *dht,           // 需要初始化对象
                              Dht11_PinWrite_t setPinStatus, // 设置总线输出值
                              Dht11_PinRead_t getPinStatus,  // 读取总线输入值
                              Dht11_SetBusMode_t mode,       // 配置总线的输入输出模式
                              Dht11_Delay_t delayms,         // 毫秒延时
                              Dht11_Delay_t delayus          // 微秒延时
);

#endif
