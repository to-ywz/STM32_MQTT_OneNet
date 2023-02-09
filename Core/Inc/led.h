#ifndef __LED_H
#define __LED_H

#include "bsp_gpio.h"

#define MACROSTR(k) #k

#define LED_MEMBERS                     \
    X(LED_None)      /* 无 */          \
    X(LED_Oneshot)   /* 闪烁一次 */ \
    X(LED_Heartbeat) /* 心跳 */       \
    X(LED_Cycle)     /* 定时 */       \
    X(LED_toON)      /* 亮 */          \
    X(LED_toOFF)     /* 灭 */

// LED 工作状态枚举
typedef enum Led_status
{
#define X(Enum) Enum,
    LED_MEMBERS
#undef X
} led_status_t;

#undef LED_MEMBERS

// LED 数量
#define LED_NUM 1

typedef struct LEDObeject
{
    uint8_t id;        // LED 编号
    uint8_t tick;      // LED 计时器(不用管)
    uint16_t pin_num;  // LED 引脚编号
    led_status_t mode; // LED 模式
} LedObeject_t;

typedef struct LedListObject
{
    LedObeject_t led[LED_NUM];

    led_status_t (*getMode)(uint8_t id);              // 获取 LED 状态
    void (*setupMode)(uint8_t id, led_status_t mode); // 设置 LED 状态
} LedListObject_t;

// LED初始化
void Led_Init(void);
void Led_CheckMode(void);

extern LedListObject_t G_led_list;

#endif
