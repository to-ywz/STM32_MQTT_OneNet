/**
 * @file SysTick.h
 * @author BlackSheep (blacksheep.208h@gmail.com)
 * @brief SysTick,规划实现软件定时器
 * @version 0.1
 * @date 2020-06-28
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#ifndef _SYSTICK_H_
#define _SYSTICK_H_

void systick_init(void);
void delayms(uint16_t nms);
void delayus(uint16_t nus);

#endif // !_SYSTICK_H_
//==================================================End=================================================
