#ifndef __BSP_GPIO_DEF_H__
#define __BSP_GPIO_DEF_H__

// 确定引脚总数
#define ZGT 

#define GPIO_PRE_PIN 16

#ifdef ZGT
    #define PIN_MAX_NUM 144
#elif  VET
    #define PIN_MAX_NUM 100
#elif  RET
    #define PIN_MAX_NUM 64
#elif  CET
    #define PIN_MAX_NUM 48
#endif

// typedef struct Pin_Type
// {
//     uint8_t port;
//     uint16_t pin;
// } Pin_Type;


#define GET_PIN(pin) pin % GPIO_PRE_PIN  // 获取引脚号
#define GET_PORT(pin) pin / GPIO_PRE_PIN // 获取端口号


#endif
