#include <stdint.h>
#include <stdio.h>
#include "stm32f4xx_hal.h"
#include "nrf24l01_ops.h"
#include "nrf24l01.h"
#include "bsp_gpio.h"
#include "delay.h"
#include "bsp_spi.h"
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
    // printf("rf24l01 is initialized.\r\n");
}

static uint8_t nrf24l01_byte_readwrite(uint8_t txdata)
{
    uint8_t rxdata = bsp_spi_readwrite0(txdata);

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
    return bsp_pin_read(get_pinnum(NRF_IRQ_PIN));
}
static void nrf24l01_delayms(volatile uint16_t ntime)
{
    delay_ms(ntime);
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
void nrf24l01_data_recv(uint8_t buf[])
{

    if (!nrf24l01_packet_recv(&nrf, buf))
    {
        if (buf[0] == 0xaa && buf[1] == 0x0f && buf[4] == 0xf0 && 0xaa == buf[5])
        {
            buf[0] = buf[2], buf[1] = buf[3];
            printf("humidity = %d.%d\r\n", buf[2], buf[3]);
        }
        else
        {
            printf("frame data is error.\r\n");
        }
    }
}

// FILE END
