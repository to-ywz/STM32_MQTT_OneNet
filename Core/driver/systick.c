/**
 * @file SysTick.c
 * @author BlackSheep (blacksheep.208h@gmail.com)
 * @brief SysTick定时器
 * @version 0.1
 * @date 2020-07-06
 *
 * @copyright Copyright (c) 2020
 *
 */
#include <stdint.h>
#include "stm32f4xx.h"
#include "stm32f4xx_it.h"
#include "sf_timer.h"

#define USE_HAL_LEGACY
#include "stm32_hal_legacy.h"

#define Timebase_Source_is_SysTick 1 // 当Timebase Source为SysTick时改为1
// #define Timebase_Source_is_SysTick 0	//当使用FreeRTOS，Timebase Source为其他定时器时改为0

#if (!Timebase_Source_is_SysTick)
extern TIM_HandleTypeDef htimx; // 当使用FreeRTOS，Timebase Source为其他定时器时，修改为对应的定时器
#define Timebase_htim htimx

#define Delay_GetCounter() __HAL_TIM_GetCounter(&Timebase_htim)
#define Delay_GetAutoreload() __HAL_TIM_GetAutoreload(&Timebase_htim)
#else
#define Delay_GetCounter() (SysTick->VAL)
#define Delay_GetAutoreload() (SysTick->LOAD)
#endif

static uint16_t fac_us = 0;
static uint32_t fac_ms = 0;

// ms 延时专用
volatile uint32_t s_uiDelayCount;
volatile uint8_t s_ucTimeOutFlag;

/*初始化*/
void systick_init(void)
{
#if (!Timebase_Source_is_SysTick)
    fac_ms = 1000000; // 作为时基的计数器时钟频率在HAL_InitTick()中被设为了1MHz
    fac_us = fac_ms / 1000;
#else
    fac_ms = SystemCoreClock / 1000;
    fac_us = fac_ms / 1000;
#endif
}

/**
 * @brief us延时
 * @note 72M条件下，nus<=1864
 *      如果超出这个值，建议多次调用此函数来实现
 *      最大延时为：nus<=0xffffff*8*1000/SYSCLK
 * @note 本 延时函数 在定时器运行的情况下进行延时
 *      不对 LOAD 进行重装载
 * @param nus nus 延时
 */
void delayus(uint32_t nus)
{
    uint32_t ticks = 0;
    uint32_t told = 0;
    uint32_t tnow = 0;
    uint32_t tcnt = 0;
    uint32_t reload = 0;

    reload = Delay_GetAutoreload();

    ticks = nus * fac_us;

    told = Delay_GetCounter();

    while (1)
    {
        tnow = Delay_GetCounter();

        if (tnow != told)
        {
            if (tnow < told)
            {
                tcnt += told - tnow;
            }
            else
            {
                tcnt += reload - tnow + told;
            }
            told = tnow;
            if (tcnt >= ticks)
            {
                break;
            }
        }
    }
}

/**
 * @brief ms延时
 * @note 72M条件下，nms<=1864
 *      如果超出这个值，建议多次调用此函数来实现
 *      最大延时为：nms<=0xffffff*8*1000/SYSCLK
 * @param nms nms 延时
 * @note 本 延时函数 在定时器运行的情况下进行延时
 *      不对 LOAD 进行重装载
 */
void delayms(uint16_t nms)
{
    uint32_t ticks = 0;
    uint32_t told = 0;
    uint32_t tnow = 0;
    uint32_t tcnt = 0;
    uint32_t reload = 0;

    reload = Delay_GetAutoreload();

    ticks = nms * fac_ms;

    told = Delay_GetCounter();

    while (1)
    {
        tnow = Delay_GetCounter();

        if (tnow != told)
        {
            if (tnow < told)
            {
                tcnt += told - tnow;
            }
            else
            {
                tcnt += reload - tnow + told;
            }
            told = tnow;
            if (tcnt >= ticks)
            {
                break;
            }
        }
    }
}

/**
 * @brief  This function handles SysTick Handler.
 * @param  None
 * @note
 * @retval None
 */
void SysTick_ISR(void)
{
    static uint8_t s_count = 0;

    if (s_uiDelayCount > 0)
    {
        if (0 == --s_uiDelayCount)
        {
            s_ucTimeOutFlag = 1;
        }
    }
    sf_timer_ticks();
}

void HAL_SYSTICK_Callback(void)
{
    SysTick_ISR();
}

/*重写HAL_Delay*/
void HAL_Delay(uint32_t Delay)
{
    uint32_t tickstart = HAL_GetTick();
    uint32_t wait = Delay;

    /*不太明白官方源码为啥这么写，会多延时1ms，注释掉后更准*/
    //  /* Add a freq to guarantee minimum wait */
    //  if (wait < HAL_MAX_DELAY)
    //  {
    //    wait += (uint32_t)(uwTickFreq);
    //  }

    while ((HAL_GetTick() - tickstart) < wait)
    {
    }
}
