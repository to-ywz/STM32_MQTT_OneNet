/**
 * @file led.c
 * @author BlackSheep (blacksheep.208h@gmail.com)
 * @brief   LED闪烁
 * @version 0.1
 * @date 2023-02-09
 *
 * @copyright Copyright (c) 2023
 *
 */
#include <stdint.h>

#include "led.h"

// 宏
#define arrayof(x) (sizeof(x) / sizeof(x[0]))

#ifdef LED_HIGH_ON
#define LED_ON 1
#define LED_OFF 0
#else
#define LED_ON 0
#define LED_OFF 1
#endif

// 闪烁周期
#define PERIOD 4

// 数据类型定义
typedef void (*led_mode_op_t)(uint8_t id);
typedef struct led_mode_info
{
  led_status_t mode;
  led_mode_op_t operation;
} led_mode_info_t;

// 本地函数
// LED 基本操作
static void Led_turnOn(uint8_t id);
static void Led_turnOff(uint8_t id);
static void Led_Toggle(uint8_t id);

// LED 工作模式
static void Led_NoneMode(uint8_t id);
static void Led_OneshotMode(uint8_t id);
static void Led_TimerMode(uint8_t id);
static void Led_HeartbeatMode(uint8_t id);

// LED 工作状态
static void setupLedMode(uint8_t id, led_status_t mode);
static led_status_t getLedMode(uint8_t id);

// 全局变量
uint16_t Pin_List[LED_NUM] = {
    89,
};
LedListObject_t G_led_list;

// 静态全局
static led_mode_info_t Led_Mode_funcs[] = {
    {LED_None, Led_NoneMode},
    {LED_Oneshot, Led_OneshotMode},
    {LED_Heartbeat, Led_HeartbeatMode},
    {LED_Cycle, Led_TimerMode},
    {LED_toON, Led_turnOn},
    {LED_toOFF, Led_turnOff}};

/**
 * @brief LED 初始化
 *
 */
void Led_Init(void)
{
  G_led_list.getMode = getLedMode;
  G_led_list.setupMode = setupLedMode;

  for (uint8_t i = 0; i < LED_NUM; i++)
  {
    G_led_list.led[i].pin_num = Pin_List[i];
    G_led_list.led[i].id = i;
    G_led_list.led[i].mode = LED_None;
    G_led_list.led[i].tick = 1;
    bsp_pin_mode(Pin_List[i], PIN_MODE_OUTPUT);
  }
  // 如果需要使某个LED处于其它模式, 可在这修改
  G_led_list.led[0].mode = LED_Heartbeat;

  Led_CheckMode();
}

/**
 * @brief               获取 LED 工作状态
 *
 * @param id            LED id 编号
 * @return led_status_t  LED 工作状态
 */
static led_status_t getLedMode(uint8_t id)
{
  return G_led_list.led[id].mode;
}

/**
 * @brief       设置 LED 工作状态
 *
 * @param id    LED id 编号
 * @param mode LED 工作状态
 */
static void setupLedMode(uint8_t id, led_status_t mode)
{
  G_led_list.led[id].mode = mode;
}

//===========================================LED ops=============================================
/**
 * @brief     点亮 LED
 *
 * @param id  LED 编号
 */
static void Led_turnOn(uint8_t id)
{
  bsp_pin_write(G_led_list.led[id].pin_num, LED_ON);
}

/**
 * @brief     开启 LED
 *
 * @param id  LED 编号
 */
static void Led_turnOff(uint8_t id)
{
  bsp_pin_write(G_led_list.led[id].pin_num, LED_OFF);
}

/**
 * @brief     LED 电平反转
 *
 * @param id  LED 编号
 */
static void Led_Toggle(uint8_t id)
{
  bsp_pin_toggle(G_led_list.led[id].pin_num);
}

/**
 * @brief     LED None 模式
 *
 * @param id  LED 编号
 */
static void Led_NoneMode(uint8_t id)
{
  Led_turnOff(id);
}

/**
 * @brief     LED 定时闪烁模式
 *
 * @param id  LED 编号
 */
static void Led_TimerMode(uint8_t id)
{
  G_led_list.led[id].tick++;
  if (G_led_list.led[id].tick < PERIOD)
  {
    return;
  }

  G_led_list.led[id].tick = 1;
  Led_Toggle(id);
}

/**
 * @brief     LED 单次闪烁模式
 *
 * @param id  LED_ID
 */
static void Led_OneshotMode(uint8_t id)
{
  static uint8_t finished = 0;
  G_led_list.led[id].tick++;
  if ((!finished) && LED_OFF == bsp_pin_read(G_led_list.led[id].pin_num))
  { // 先熄灭 LED
    Led_turnOff(id);
    return;
  }
  if ((!finished) && G_led_list.led[id].tick == 4)
  {                 // 200ms 内 不点亮 LED
    Led_turnOn(id); // 点亮 LED
  }

  if ((!finished) && G_led_list.led[id].tick == 14)
  { // 500ms 后LED 熄灭, 不再进行闪烁操作
    G_led_list.led[id].tick = 1;
    Led_turnOff(id);
    G_led_list.setupMode(id, LED_None); // 在这个工程中的特例
    //  finished = 1;
  }
}

/**
 * @brief     LED 心跳模式
 *
 * @param id  LED 编号
 */
static void Led_HeartbeatMode(uint8_t id)
{
  if (!G_led_list.led[id].tick)
  { // 先关闭 LED, 保证 4次取反之后为关闭状态
    Led_turnOff(id);
  }

  G_led_list.led[id].tick++;
  if (0 == G_led_list.led[id].tick % 2 && // 100ms 翻转一次
      G_led_list.led[id].tick <= 8)       // 4次反转(400ms)后长灭
  {                                       // 无需特判 0, 进入该分支判断时tick最小值为 1
    Led_Toggle(id);
    return;
  }

  if (22 == G_led_list.led[id].tick)
  { //
    G_led_list.led[id].tick = 1;
  }
}

/**
 * @brief     LED 工作模式检查
 *
 * !@note     中断服务函数 中 50ms一次
 */
void Led_CheckMode(void)
{
  for (uint8_t i = 0; i < LED_NUM; i++)
  { // 遍历 LED
    Led_Mode_funcs[G_led_list.led[i].mode].operation(i);
  }
}
//==================================================End=================================================
