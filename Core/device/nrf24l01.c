#include <stdint.h>
#include <stdio.h>
#include "stm32f4xx_hal.h"
#include "nrf24l01_ops.h"
#include "nrf24l01.h"
#include "bsp_gpio.h"
#include "systick.h"
#include "spi.h"
#include "sys.h"

static NRF24L01ObjectType nrf;

static uint8_t NRF24L01ReadWriteByte(uint8_t txdata); // 声明向nRF24L01读写一个字节的函数
static void NRF24L01ChipSelect(NRF24L01CSType cs);    // 声明片选操作函数
static void NRF24L01ChipEnable(NRF24L01CEType en);    // 声明使能及模式操作函数
static uint8_t NRF24L01GetIRQ(void);                  // 声明中断获取函数
static void NRF24L01Delayms(volatile uint32_t ntime); // 毫秒延时操作指针

/**
 * @brief   nrf 对象初始化
 *
 */
void nrf24l01_init(void)
{
    if (NRF24L01_NoError != NRF24L01Initialization(&nrf,
                                                   NRF24L01ReadWriteByte,
                                                   NRF24L01ChipSelect,
                                                   NRF24L01ChipEnable,
                                                   NRF24L01GetIRQ,
                                                   NRF24L01Delayms))
    {
        printf("nrf24l01 init is failed.\r\n");
        while (1)
            ;
    }
}

static uint8_t NRF24L01ReadWriteByte(uint8_t txdata)
{
    uint8_t rxdata = 0;
    HAL_SPI_TransmitReceive(&hspi1, &txdata, &rxdata, 1, 1000);

    return rxdata;
}
static void NRF24L01ChipSelect(NRF24L01CSType cs)
{
    bsp_pin_write(get_pinnum(NRF_CS_PIN), cs);
}
static void NRF24L01ChipEnable(NRF24L01CEType en) // ce
{
    bsp_pin_write(get_pinnum(NRF_CE_PIN), en);
}
static uint8_t NRF24L01GetIRQ(void)
{
    return bsp_pin_read(NRF_IQR_Pin);
}
static void NRF24L01Delayms(volatile uint32_t ntime)
{
    delayms(ntime);
}
#include "stdio.h"
/*NRF24L01数据通讯*/
void NRF24L01DataExchange(void)
{
    uint8_t txDatas[32] = {0xAA};
    uint8_t rxDatas[32] = {0x00};

    if (0x20 == NRF24L01TransmitPacket(&nrf, txDatas))
    {
        printf("nrf is send data successs\r\n");
    }
    else
    {
        printf("nrf is send data failed\r\n");
    }
    // NRF24L01ReceivePacket(&nrf, rxDatas);
    // printf("%x\r\n", rxDatas[0]);
}

// FILE END
