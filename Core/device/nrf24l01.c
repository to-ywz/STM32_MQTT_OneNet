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
    return bsp_pin_read(get_pinnum(NRF_IQR_PIN));
}
static void nrf24l01_delayms(volatile uint16_t ntime)
{
    delayms(ntime);
}
#include "stdio.h"

/**
 * @brief       发送数据业务逻辑
 *
 */
void nrf24l01_data_xmit(float data)
{
    uint8_t txbuf[32] = {0xAA, 0x0f, 0, 0, 0xf0, 0xAA};

    txbuf[2] = (int)(data);
    txbuf[3] = (data - (int)(data)) * 100;

    if (0x20 == nrf24l01_packet_xmit(&nrf, txbuf))
    {
        printf("send success\r\n");
    }
    else
    {
        printf("send failed\r\n");
    }
}

/**
 * @brief   接收数据业务逻辑
 *
 */
void nrf24l01_data_recv(void)
{
    uint8_t rxDatas[32] = {0x00};

    if (!nrf24l01_packet_recv(&nrf, rxDatas))
    {
        if (rxDatas[0] == 0xaa && rxDatas[1] == 0x0f && rxDatas[4] == 0xf0 && 0xaa == rxDatas[5])
        {
            printf("humidity = %d.%d\r\n", rxDatas[2], rxDatas[3]);
        }
        else
        {
            printf("frame data is error.\r\n");
        }
    }
}

// FILE END
