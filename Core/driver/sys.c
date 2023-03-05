/**
 * @file sys.c
 * @author BlackSheep (blacksheep.208h@gmail.com)
 * @brief   通用库函数(例如itoa)
 * @version 0.1
 * @date 2023-03-02
 *
 * @copyright Copyright (c) 2023
 *
 */
#include <stdio.h>
#include <string.h>
#include "sys.h"
#include "stm32f4xx_hal.h"

/**
 * @brief       获得引脚编号
 *
 * @param str   类似PA10类型的字符
 * @retval      引脚编号(-1为非法)
 */
int get_pinnum(char *str)
{
    // 将字符转为小写
    str[0] |= 0x20, str[1] |= 0x20;
    if (str[0] != 'p')
        return -1;

    int len = strlen(str);
    int pin = 0, port = 0;

    if (len != 3 && len != 4)
        return -1;

    port = str[1] - 'a';
    pin = str[2] - '0';
    if (len > 3)
        pin = pin * 10 + str[3] - '0';

    return (port * 16 + pin);
}

/**
 * @brief 系统复位函数
 *
 */
void board_system_reset(void)
{
    __set_FAULTMASK(1); // 关闭所有中断
    NVIC_SystemReset(); // 复位
}

/**
 * @brief           将整形数据转换成字符串
 *
 * @param value     要转换的整形数
 * @param string    转换后的字符串
 * @param radix     进制
 * @retval          转换后的字符串
 */
char *itoa(int value, char *string, int radix)
{
    int i, d;
    int flag = 0;
    char *ptr = string;

    /* This implementation only works for decimal numbers. */
    if (radix != 10)
    {
        *ptr = 0;
        return string;
    }

    if (!value)
    {
        *ptr++ = 0x30;
        *ptr = 0;
        return string;
    }

    /* if this is a negative value insert the minus sign. */
    if (value < 0)
    {
        *ptr++ = '-';

        /* Make the value positive. */
        value *= -1;
    }

    for (i = 10000; i > 0; i /= 10)
    {
        d = value / i;

        if (d || flag)
        {
            *ptr++ = (char)(d + 0x30);
            value -= (d * i);
            flag = 1;
        }
    }

    /* Null terminate the string. */
    *ptr = 0;

    return string;

} /* NCL_Itoa */