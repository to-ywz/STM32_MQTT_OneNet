#include <stdint.h>
#include "stm32f4xx_hal.h"
#include "bootloader.h"
#include "bsp_flash.h"

#define APP1_START_ADDRESS ((uint32_t)0x08004000)       // 应用程序的起始地址
#define APP2_START_ADDRESS ((uint32_t)0x800C000 + 0x10) // 应用程序的起始地址
#define APP_SIZE (128 * 1024)                           // 应用程序大小为128KB

/**
 * @brief 写入应用程序代码到Flash存储器, 并擦除原有应用程序代码
 *
 * @param[in] app_data 应用程序代码的指针
 * @param[in] app_size 应用程序代码的大小
 * @return  返回0表示写入成功
 *          返回1表示擦除失败
 *          返回2表示写入失败
 */
uint32_t iap_write_app(uint8_t *app_data, uint32_t app_size)
{
    return flash_write_data(APP2_START_ADDRESS, app_data, app_size);
}

/**
 * @brief 从Flash存储器中读取应用程序代码数据
 *
 * @param[in]  address 读取的起始地址
 * @param[out] data    读取数据的指针
 * @param[in]  size    要读取的数据的大小
 * @return 返回0表示读取成功, 返回其他值表示读取失败
 */
uint32_t iap_read_app(uint8_t *app_data, uint32_t app_size)
{
    return flash_read_data(APP2_START_ADDRESS, app_data, app_size);
}

/**
 * @brief 从当前位置跳转到应用程序代码的入口点, 以便启动应用程序
 */
void iap_jump_to_app(void)
{
    HAL_DeInit();
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

    uint32_t jump_address = *(volatile uint32_t *)(APP1_START_ADDRESS + 4);
    void (*app_entry)(void) = (void (*)(void))jump_address;
    __set_MSP(*(volatile uint32_t *)APP1_START_ADDRESS);
    app_entry();
}

// FILE END
