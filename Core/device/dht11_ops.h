/**
 * @file dht11_ops.h
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
#ifndef __DHT11_OPS_H
#define __DHT11_OPS_H

#include "stdint.h"

/*定义DHT11错误消息枚举*/
typedef enum dht11_error
{
    DHT11_NOERROR,   // 没有错误
    DHT11_NONE,      // 未检测到DHT11
    DHT11_INITERROR, // 初始化错误
    DHT11_DATAERROR  // 通讯校验错误
} dht11_error_et;

/*定义单总线方向枚举*/
typedef enum dht11_io_mode
{
    DHT11_IN = 0,
    DHT11_OUT = 1
} dht11_busmode_et;

/*定义单总线操作值枚举*/
typedef enum dht11_pinvalue
{
    DHT11_RESET = 0,
    DHT11_SET = 1
} dht11_busvalue_et;

/* 定义DHT11对象类型 */
typedef struct dht11_struct
{
    float temperature; // 温度值
    float humidity;    // 湿度值

    void (*set_Pin_value)(dht11_busvalue_et value); // 设置DHT11引脚的输出值
    uint8_t (*read_pin_value)(void);                // 读取引脚电平
    void (*set_pin_mode)(dht11_busmode_et mode);    // 设置引脚的输入输出方向

    void (*delayms)(volatile uint16_t ntime); // 实现ms延时操作
    void (*delayus)(volatile uint16_t ntime); // 实现us延时操作
} dht11_object_t;

typedef void (*dht11_pinwrite_t)(dht11_busvalue_et value);
/*定义读取引脚电平函数类型*/
typedef uint8_t (*dht11_pinread_t)(void);
/*定义设置引脚的输入输出方向函数类型*/
typedef void (*dht11_setbusmode_t)(dht11_busmode_et mode);
/*定义实现ms延时操作函数类型*/
typedef void (*dht11_delay_t)(volatile uint16_t ntime);


dht11_error_et dht11_value_read(dht11_object_t *dht);
dht11_error_et dht11_ops_init(dht11_object_t *dht,
                              dht11_pinwrite_t pinstatus_setup,
                              dht11_pinread_t pinstatus_read,
                              dht11_setbusmode_t mode,
                              dht11_delay_t delayms,
                              dht11_delay_t delayus);

#endif
//==================================================End=================================================
