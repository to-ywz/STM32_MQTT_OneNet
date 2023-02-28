/**
 * @file sf_timer.h
 * @author  秦殇 (you@domain.com)
 * @brief 软件定时器
 * @version 0.1
 * @date 2020-07-15
 *
 * @copyright Copyright (c) 2020
 *
 */
#ifndef _SF_TIMER_H_
#define _SF_TIMER_H_

#include "stdint.h"
#include "stddef.h"

//---------------------- 头文件 -------------------------
/* 芯片头文件 放置 */
// #include "stm32f10x.h"

/* 用户头文件 */
// #include "SysTick.h"

//------------------- 数据类型定义 -----------------------
typedef struct sf_timer_t
{
    uint32_t timeout;         // 超时时间, _timer_tick+timout;
    uint32_t repeat;          // 重复次数
    void (*timeout_cb)(void); // 超时回调函数,中断服务
    struct sf_timer_t *next;       //
} sf_timer_t;



void sf_timer_init(sf_timer_t *handle, void (*timeout_cb)(void), uint32_t timeout, uint32_t repeat);
int sf_timer_start(sf_timer_t *handle);
void sf_timer_stop(sf_timer_t *handle);
void sf_timer_ticks(void);
void sf_timer_loop(void);

// 定时 服务函数
void timer1_callback(void);
void timer2_callback(void);

#endif
//========================================================== END OF FILE ==========================================================
