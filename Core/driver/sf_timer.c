#include <stdio.h>
#include "sf_timer.h"
#include "stm32f4xx_hal.h"
#include "led.h"

#define ENABLE_INT() __set_PRIMASK(0)  /* 使能全局中断 */
#define DISABLE_INT() __set_PRIMASK(1) /* 禁止全局中断 */

/**
 * @file soft_timer.c
 * @author  秦殇 (you@domain.com)
 * @brief   软件定时器 逻辑实现
 * @version 0.1
 * @date 2020-07-15
 *
 * @copyright Copyright (c) 2020
 *
 */

#include "sf_timer.h"


// 软件定时器工作列表.
static sf_timer_t *head_handle = NULL;

// 定时器定时 全局变量 最大值为 FFFFFFFF,注意溢出
// 理论最大计时天数为 49.71 天
static uint32_t _timer_ticks = 0;

/**
 * @brief  初始化定时器结构体.
 * @param  handle: 需要初始化的 软件定时器结构体.
 * @param  timeout_cb: 回调函数.
 * @param  timeout: 触发时间
 * @param  repeat: 重复间隔(重装载时间)
 *
 * @note   作为延时函数, repeat 置 0
 *     自带无限循环
 *
 * @retval None
 */
void sf_timer_init(sf_timer_t *handle, void (*timeout_cb)(void), uint32_t timeout, uint32_t repeat)
{
    handle->timeout_cb = timeout_cb;
    DISABLE_INT();
    handle->timeout = _timer_ticks + timeout;
    ENABLE_INT();
    handle->repeat = repeat;
}

/**
 * @brief  使能软件定时器 将定时器添加到 工作表中.
 * @param  handle: 需要使能的 软件定时器.
 * @retval 0: 使能能成功. -1: 已存在.
 */
int sf_timer_start(sf_timer_t *handle)
{
    sf_timer_t *target = head_handle;
    while (target)
    {
        if (target == handle)
        {
            return -1;
        } // 已存在.
        target = target->next;
    }
    handle->next = head_handle;
    head_handle = handle;
    return 0;
}

/**
 * @brief  停止软件定时器 将定时器移除 工作表.
 * @param  handle: 需要移除的 软件定时器.
 * @retval None
 */
void sf_timer_stop(sf_timer_t *handle)
{
    sf_timer_t **curr;
    for (curr = &head_handle; *curr;)
    {
        sf_timer_t *entry = *curr;
        if (entry == handle)
        {
            *curr = entry->next;
        }
        else
        {
            curr = &entry->next;
        }
    }
}

/**
 * @brief  检测,可以扔到定时器中,也可以放到循环.
 * @param  None.
 * @retval None
 */
void sf_timer_loop()
{
    sf_timer_t *target;
    for (target = head_handle; target; target = target->next)
    {
        if (_timer_ticks >= target->timeout)
        { // 超时判定
            if (target->repeat == 0)
            { // 停止 定时器
                sf_timer_stop(target);
            }
            else
            {
                DISABLE_INT();
                target->timeout = _timer_ticks + target->repeat;
                ENABLE_INT();
            }
            target->timeout_cb(); // 执行回调
        }
    }
}

/**
 * @brief  1ms 调用一次,负责计时.
 * @param  None.
 * @retval None.
 */
void sf_timer_ticks()
{
    DISABLE_INT();
    _timer_ticks++;
    ENABLE_INT();
}

//------------------------ 软件定时器中断服务函数 -----------------------------
// * 可以在主函数中实现 也可以在此处实现,无需删除

__weak void timer1_callback()
{
    // TODO
}

__weak void timer2_callback()
{
    // TODO
}

//========================================================== END OF FILE ==========================================================
