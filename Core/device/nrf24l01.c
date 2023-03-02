#include "nrf24l01.h"
#include "systick.h"
#include "stm32f4xx_hal.h"
#include "spi.h"

static NRF24L01ObjectType nrf24l01;

static uint8_t NRF24L01ReadWriteByte(uint8_t TxData); // 声明向nRF24L01读写一个字节的函数
static void NRF24L01ChipSelect(NRF24L01CSType cs);    // 声明片选操作函数
static void NRF24L01ChipEnable(NRF24L01CEType en);    // 声明使能及模式操作函数
static uint8_t NRF24L01GetIRQ(void);                  // 声明中断获取函数
static void NRF24L01Delayms(volatile uint32_t nTime); // 毫秒延时操作指针

/**
 * @brief   nrf 对象初始化
 *
 */
void hw_nrf_init(void)
{
    NRF24L01Initialization(&nrf,
                           NRF24L01ReadWriteByte,
                           NRF24L01ChipSelect,
                           NRF24L01ChipEnable,
                           NRF24L01GetIRQ,
                           NRF24L01Delayms);
}

static uint8_t NRF24L01ReadWriteByte(uint8_t TxData);
{
    HAL_SPI_TransmitReceive(&hspi1, );
}
static void NRF24L01ChipSelect(NRF24L01CSType cs); // 声明片选操作函数
static void NRF24L01ChipEnable(NRF24L01CEType en); // 声明使能及模式操作函数
static uint8_t NRF24L01GetIRQ(void);               // 声明中断获取函数
static void NRF24L01Delayms(volatile uint32_t nTime)
{
    delayms(nTime);
}

// FILE END
