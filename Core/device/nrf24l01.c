#include <stdint.h>
#include <stdio.h>
#include "stm32f4xx_hal.h"
#include "nrf24l01_ops.h"
#include "nrf24l01.h"
#include "bsp_gpio.h"
#include "systick.h"
#include "spi.h"
#include "sys.h"

static nrf24l01_t nrf;

static uint8_t nrf24l01_byte_readwrite(uint8_t txdata); // 声明向nRF24L01读写一个字节的函数
static void nrf24l01_chip_select(NRF24L01_CS_ET cs);    // 声明片选操作函数
static void nrf24l01_chip_enable(NRF24L01_CE_ET en);    // 声明使能及模式操作函数
static uint8_t nrf24l01_iqr_get(void);                  // 声明中断获取函数
static void nrf24l01_delayms(volatile uint16_t ntime);  // 毫秒延时操作指针

/**
 * @brief   nrf 对象初始化
 *
 */
void nrf24l01_init(void)
{
    if (NRF24L01_NOERROR != nrf24L01_init(&nrf,
                                          nrf24l01_byte_readwrite,
                                          nrf24l01_chip_select,
                                          nrf24l01_chip_enable,
                                          nrf24l01_iqr_get,
                                          nrf24l01_delayms))
    {
        printf("nrf24l01 init is failed.\r\n");
        while (1)
            ;
    }
}

static uint8_t nrf24l01_byte_readwrite(uint8_t txdata)
{
    uint8_t rxdata = 0;
    HAL_SPI_TransmitReceive(&hspi1, &txdata, &rxdata, 1, 1000);

    return rxdata;
}
static void nrf24l01_chip_select(NRF24L01_CS_ET cs)
{
    bsp_pin_write(get_pinnum(NRF_CS_PIN), cs);
}
static void nrf24l01_chip_enable(NRF24L01_CE_ET en) // ce
{
    bsp_pin_write(get_pinnum(NRF_CE_PIN), en);
}
static uint8_t nrf24l01_iqr_get(void)
{
    return bsp_pin_read(NRF_IQR_Pin);
}
static void nrf24l01_delayms(volatile uint16_t ntime)
{
    delayms(ntime);
}
#include "stdio.h"
/*NRF24L01数据通讯*/
/*NRF24L01数据通讯*/
void NRF24L01DataExchange(void)
{
    uint8_t txDatas[32] = {0xAA};
    uint8_t rxDatas[32] = {0x00};

    // 发送
    // if (0x20 == nrf24l01_packet_xmit(&nrf, txDatas))
    // {
    //     printf("send success\r\n");
    // }
    // else
    // {
    //     printf("send failed\r\n");
    // }
    // 接收
    if (!nrf24l01_packet_recv(&nrf, rxDatas))
    {
        for (int i = 0; i < 5; i++)
        {
            printf("%x ", rxDatas[i]);
        }
        printf("\r\n");
    }
}

// FILE END
