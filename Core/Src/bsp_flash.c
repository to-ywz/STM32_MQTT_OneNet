#include "bsp_flash.h"
#include "stm32f4xx_hal.h"

#define FLASH_PAGE_SIZE 1024

/**
 * @brief 从RAM中写入数据到Flash存储器
 *
 * @param[in] address 要写入的地址
 * @param[in] data 数据
 * @param[in] size 数据大小
 * @return  返回0表示写入成功
 *          返回1表示擦除失败
 *          返回2表示写入失败
 */
uint32_t flash_write_data(uint32_t address, uint8_t *data, uint32_t size)
{
    HAL_StatusTypeDef status;
    FLASH_EraseInitTypeDef erase_init;
    uint32_t page_errors = 0;
    uint32_t i;

    HAL_FLASH_Unlock();

    erase_init.TypeErase = FLASH_TYPEERASE_SECTORS;
    erase_init.Sector = FLASH_SECTOR_7;
    erase_init.NbSectors = 1;
    erase_init.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    status = HAL_FLASHEx_Erase(&erase_init, &page_errors);
    if (status != HAL_OK)
    {
        HAL_FLASH_Lock();
        return 1;
    }

    for (i = 0; i < size; i++)
    {
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, address++, data[i]);
        if (status != HAL_OK)
        {
            HAL_FLASH_Lock();
            return 2;
        }
    }

    HAL_FLASH_Lock();

    return 0;
}

/**
 * @brief 从Flash存储器中读取数据
 *
 * @param[in]  address 读取的起始地址
 * @param[out] data    读取数据的指针
 * @param[in]  size    要读取的数据的大小
 * @return 返回0表示读取成功，返回其他值表示读取失败
 */
uint32_t flash_read_data(uint32_t address, uint8_t *data, uint32_t size)
{
    HAL_StatusTypeDef status;

    for (uint32_t i = 0; i < size; i++)
    {
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, address + i, data[i]);
        if (status != HAL_OK)
        {
            return 1;
        }
    }

    return 0;
}
