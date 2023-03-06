/**
 * @file nrf24l01_ops.c
 * @author BlackSheep (blacksheep.208h@gmail.com)
 * @brief   用于实现nRF24L01无线通讯模块的操作 
 *          nRF24L01的命令格式：指令+数据
 *          (注释太乱可以去nrf_ops.md中查看) 
 * @version 0.1
 * @date 2023-03-06
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "stddef.h"
#include "nrf24l01_ops.h"

/*nRF24L01寄存器操作命令*/
#define READ_REG_NRF 0x00  // 读配置寄存器,低5位为寄存器地址
#define WRITE_REG_NRF 0x20 // 写配置寄存器,低5位为寄存器地址
#define RD_RX_PLOAD 0x61   // 读RX有效数据,1~32字节
#define WR_TX_PLOAD 0xA0   // 写TX有效数据,1~32字节
#define FLUSH_TX 0xE1      // 清除TX FIFO寄存器.发射模式下用
#define FLUSH_RX 0xE2      // 清除RX FIFO寄存器.接收模式下用
#define REUSE_TX_PL 0xE3   // 重新使用上一包数据,CE为高,数据包被不断发送.
#define NOP 0xFF           // 空操作,可以用来读状态寄存器

/*nRF24L01寄存器地址*/
#define CONFIG 0x00          // 配置寄存器地址;bit0:1接收模式,0发射模式;bit1:电选择;bit2:CRC模式;bit3:CRC使能;
                             // bit4:中断MAX_RT(达到最大重发次数中断)使能;bit5:中断TX_DS使能;bit6:中断RX_DR使能
#define EN_AA 0x01           // 使能自动应答功能  bit0~5,对应通道0~5
#define EN_RXADDR 0x02       // 接收地址允许,bit0~5,对应通道0~5
#define SETUP_AW 0x03        // 设置地址宽度(所有数据通道):bit1,0:00,3字节;01,4字节;02,5字节;
#define SETUP_RETR 0x04      // 建立自动重发;bit3:0,自动重发计数器;bit7:4,自动重发延时 250*x+86us
#define RF_CH 0x05           // RF通道,bit6:0,工作通道频率;
#define RF_SETUP 0x06        // RF寄存器;bit3:传输速率(0:1Mbps,1:2Mbps);bit2:1,发射功率;bit0:低噪声放大器增益
#define STATUS 0x07          // 状态寄存器;bit0:TX FIFO满标志;bit3:1,接收数据通道号(最大:6);bit4,达到最多次重发
                             // bit5:数据发送完成中断;bit6:接收数据中断;
#define OBSERVE_TX 0x08      // 发送检测寄存器,bit7:4,数据包丢失计数器;bit3:0,重发计数器
#define CD 0x09              // 载波检测寄存器,bit0,载波检测;
#define RX_ADDR_P0 0x0A      // 数据通道0接收地址,最大长度5个字节,低字节在前
#define RX_ADDR_P1 0x0B      // 数据通道1接收地址,最大长度5个字节,低字节在前
#define RX_ADDR_P2 0x0C      // 数据通道2接收地址,最低字节可设置,高字节,必须同RX_ADDR_P1[39:8]相等;
#define RX_ADDR_P3 0x0D      // 数据通道3接收地址,最低字节可设置,高字节,必须同RX_ADDR_P1[39:8]相等;
#define RX_ADDR_P4 0x0E      // 数据通道4接收地址,最低字节可设置,高字节,必须同RX_ADDR_P1[39:8]相等;
#define RX_ADDR_P5 0x0F      // 数据通道5接收地址,最低字节可设置,高字节,必须同RX_ADDR_P1[39:8]相等;
#define TX_ADDR 0x10         // 发送地址(低字节在前),ShockBurst模式下,RX_ADDR_P0与此地址相等
#define RX_PW_P0 0x11        // 接收数据通道0有效数据宽度(1~32字节),设置为0则非法
#define RX_PW_P1 0x12        // 接收数据通道1有效数据宽度(1~32字节),设置为0则非法
#define RX_PW_P2 0x13        // 接收数据通道2有效数据宽度(1~32字节),设置为0则非法
#define RX_PW_P3 0x14        // 接收数据通道3有效数据宽度(1~32字节),设置为0则非法
#define RX_PW_P4 0x15        // 接收数据通道4有效数据宽度(1~32字节),设置为0则非法
#define RX_PW_P5 0x16        // 接收数据通道5有效数据宽度(1~32字节),设置为0则非法
#define NRF_FIFO_STATUS 0x17 // FIFO状态寄存器;bit0,RX FIFO寄存器空标志;bit1,RX FIFO满标志;bit2,3,保留
                             // bit4,TX FIFO空标志;bit5,TX FIFO满标志;bit6,1,循环发送上一数据包.0,不循环;

/*定义操作状态*/
#define MAX_TX 0x10 // 达到最大发送次数中断
#define TX_OK 0x20  // TX发送完成中断
#define RX_OK 0x40  // 接收到数据中断

/*24L01发送接收数据宽度定义*/
#define TX_ADR_WIDTH 5    // 5字节的地址宽度
#define RX_ADR_WIDTH 5    // 5字节的地址宽度
#define TX_PLOAD_WIDTH 32 // 32字节的用户数据宽度
#define RX_PLOAD_WIDTH 32 // 32字节的用户数据宽度

const uint8_t TX_ADDRESS[TX_ADR_WIDTH] = {0x34, 0x43, 0x10, 0x10, 0x01}; // 发送地址
const uint8_t RX_ADDRESS[RX_ADR_WIDTH] = {0x34, 0x43, 0x10, 0x10, 0x01}; // 发送地址

/*写寄存器*/
static uint8_t nrf24l01_reg_write(nrf24l01_t *nrf, uint8_t reg, uint8_t value);
/*读取寄存器值*/
static uint8_t nrf24l01_reg_read(nrf24l01_t *nrf, uint8_t reg);
/*在指定位置读出指定长度的数据*/
static uint8_t nrf24l01_buf_read(nrf24l01_t *nrf, uint8_t reg, uint8_t *pBuf, uint8_t len);
/*在指定位置写指定长度的数据*/
static uint8_t nrf24l01_buf_write(nrf24l01_t *nrf, uint8_t reg, uint8_t *pBuf, uint8_t len);
/*检测24L01是否存在,返回值:0，成功;1，失败*/
static uint8_t nrf24l01_check(nrf24l01_t *nrf);
/*设置nRF24L01的模式*/
static void nrf24l01_mode_setup(nrf24l01_t *nrf, NRF24L01ModeType mode);
/*缺省片选处理函数*/
static void nrf24l01_cs_default(NRF24L01_CS_ET cs);

// 暂时存放, 后期放入 bsp_spi 文件中
#include "stm32f4xx_hal.h"
#include "spi.h"
void spi1_speed_setup(uint8_t baudrate_prescaler)
{
    assert_param(IS_SPI_BAUDRATE_PRESCALER(baudrate_prescaler)); // 判断有效性
    __HAL_SPI_DISABLE(&hspi1);                                      // 关闭SPI
    hspi1.Instance->CR1 &= 0XFFC7;                                  // 位3-5清零，用来设置波特率
    hspi1.Instance->CR1 |= baudrate_prescaler;                   // 设置SPI速度
    __HAL_SPI_ENABLE(&hspi1);                                       // 使能SPI
}

/*启动NRF24L01发送一次数据包*/
/*参数：txbuf:待发送数据首地址*/
/*返回值：发送完成状况*/
uint8_t nrf24l01_packet_xmit(nrf24l01_t *nrf, uint8_t *txbuf)
{
    uint8_t status;

    nrf24l01_mode_setup(nrf, NRF24L01TxMode);
    spi1_speed_setup(SPI_BAUDRATEPRESCALER_16); // spi速度为5.75Mhz（24L01的最大SPI时钟为10Mhz）

    nrf->chip_enable(NRF24L01CE_DISABLE);
    nrf24l01_buf_write(nrf, WR_TX_PLOAD, txbuf, TX_PLOAD_WIDTH); // 写数据到TX BUF 32个字节
    nrf->chip_enable(NRF24L01CE_ENABLE);                           // 启动发送

    while (nrf->get_iqr() != 0)
        ;                                                       // 等待发送完成
    status = nrf24l01_reg_read(nrf, STATUS);                // 读取状态寄存器的值
    nrf24l01_reg_write(nrf, WRITE_REG_NRF + STATUS, status); // 清除TX_DS或MAX_RT中断标志
    if (status & MAX_TX)                                        // 达到最大重发次数
    {
        nrf24l01_reg_write(nrf, FLUSH_TX, 0xFF); // 清除TX FIFO寄存器
        return MAX_TX;
    }
    if (status & TX_OK) // 发送完成
    {
        return TX_OK;
    }
    return 0xFF; // 其他原因发送失败
}

/*启动NRF24L01接收一次数据包*/
/*参数：txbuf:待发送数据首地址*/
/*返回值:0，接收完成；其他，错误代码*/
uint8_t nrf24l01_packet_recv(nrf24l01_t *nrf, uint8_t *rxbuf)
{
    uint8_t status;

    nrf24l01_mode_setup(nrf, NRF24L01RxMode);
    spi1_speed_setup(SPI_BAUDRATEPRESCALER_16);

    status = nrf24l01_reg_read(nrf, STATUS);                // 读取状态寄存器的值
    nrf24l01_reg_write(nrf, WRITE_REG_NRF + STATUS, status); // 清除TX_DS或MAX_RT中断标志
    if (status & RX_OK)                                         // 接收到数据
    {
        nrf24l01_buf_read(nrf, RD_RX_PLOAD, rxbuf, RX_PLOAD_WIDTH); // 读取数据
        nrf24l01_reg_write(nrf, FLUSH_RX, 0xFF);                  // 清除RX FIFO寄存器
        return 0;
    }
    return 1; // 没收到任何数据
}

/*nRF24L01对象初始化函数*/
NRF24L01_ERROR_ET nrf24L01_init(nrf24l01_t *nrf,              // nRF24L01对象
                                         nrf24l01_byte_readwrite_t spiReadWrite, // SPI读写函数指针
                                         nrf24l01_chip_select_t cs,              // 片选信号操作函数指针
                                         nrf24l01_chip_enable_t ce,              // 使能信号操作函数指针
                                         nrf24l01_get_iqr_t irq,                 // 中断信号获取函数指针
                                         nrf24l01_delayms_t delayms             // 毫秒延时
)
{
    int retry = 0;

    if ((nrf == NULL) || (spiReadWrite == NULL) || (ce == NULL) || (irq == NULL) || (delayms == NULL))
    {
        return NRF24L01_INITERROR;
    }
    nrf->byte_readwrite = spiReadWrite;
    nrf->chip_enable = ce;
    nrf->get_iqr = irq;
    nrf->delayms = delayms;

    if (cs != NULL)
    {
        nrf->chip_select = cs;
    }
    else
    {
        nrf->chip_select = nrf24l01_cs_default;
    }

    while (nrf24l01_check(nrf) && (retry < 5))
    {
        nrf->delayms(300);
        retry++;
    }

    if (retry >= 5)
    {
        return NRF24L01_ABSENT;
    }

    for (int i = 0; i < 8; i++)
    {
        nrf->reg[i] = 0;
    }

    nrf24l01_mode_setup(nrf, NRF24L01RxMode);

    return NRF24L01_NOERROR;
}

/*设置nRF24L01的模式*/
static void nrf24l01_mode_setup(nrf24l01_t *nrf, NRF24L01ModeType mode)
{
    nrf->chip_enable(NRF24L01CE_DISABLE);

    if (mode == NRF24L01RxMode)
    {
        /*初始化NRF24L01到RX模式。设置RX地址,写RX数据宽度,选择RF频道,波特率和LNA HCURR；
          当CE变高后,即进入RX模式,并可以接收数据了*/
        nrf24l01_buf_write(nrf, WRITE_REG_NRF + RX_ADDR_P0, (uint8_t *)RX_ADDRESS, RX_ADR_WIDTH); // 写RX节点地址

        nrf24l01_reg_write(nrf, WRITE_REG_NRF + EN_AA, 0x01);              // 使能通道0的自动应答
        nrf24l01_reg_write(nrf, WRITE_REG_NRF + EN_RXADDR, 0x01);          // 使能通道0的接收地址
        nrf24l01_reg_write(nrf, WRITE_REG_NRF + RF_CH, 40);                // 设置RF通信频率
        nrf24l01_reg_write(nrf, WRITE_REG_NRF + RX_PW_P0, RX_PLOAD_WIDTH); // 选择通道0的有效数据宽度
        nrf24l01_reg_write(nrf, WRITE_REG_NRF + RF_SETUP, 0x0F);           // 设置TX发射参数,0db增益,2Mbps,低噪声增益开启
        nrf24l01_reg_write(nrf, WRITE_REG_NRF + CONFIG, 0x0F);             // 配置基本工作模式的参数;PWR_UP,EN_CRC,16BIT_CRC,接收模式
    }
    else
    {
        /*初始化NRF24L01到TX模式。设置TX地址,写TX数据宽度,设置RX自动应答的地址,
          填充TX发送数据,选择RF频道,波特率和LNA HCURR；PWR_UP,CRC使能；当CE变高后,
          即进入RX模式,并可以接收数据了；CE为高大于10us,则启动发送*/
        nrf24l01_buf_write(nrf, WRITE_REG_NRF + TX_ADDR, (uint8_t *)TX_ADDRESS, TX_ADR_WIDTH);    // 写TX节点地址
        nrf24l01_buf_write(nrf, WRITE_REG_NRF + RX_ADDR_P0, (uint8_t *)RX_ADDRESS, RX_ADR_WIDTH); // 设置TX节点地址,主要为了使能ACK

        nrf24l01_reg_write(nrf, WRITE_REG_NRF + EN_AA, 0x01);      // 使能通道0的自动应答
        nrf24l01_reg_write(nrf, WRITE_REG_NRF + EN_RXADDR, 0x01);  // 使能通道0的接收地址
        nrf24l01_reg_write(nrf, WRITE_REG_NRF + SETUP_RETR, 0x1A); // 设置自动重发间隔时间:500us + 86us;最大自动重发次数:10次
        nrf24l01_reg_write(nrf, WRITE_REG_NRF + RF_CH, 40);        // 设置RF通道为40
        nrf24l01_reg_write(nrf, WRITE_REG_NRF + RF_SETUP, 0x0F);   // 设置TX发射参数,0db增益,2Mbps,低噪声增益开启
        nrf24l01_reg_write(nrf, WRITE_REG_NRF + CONFIG, 0x0E);     // 配置基本工作模式的参数;PWR_UP,EN_CRC,16BIT_CRC,接收模式,开启所有中断
    }

    nrf->reg[CONFIG] = nrf24l01_reg_read(nrf, CONFIG);
    nrf->reg[EN_AA] = nrf24l01_reg_read(nrf, EN_AA);
    nrf->reg[EN_RXADDR] = nrf24l01_reg_read(nrf, EN_RXADDR);
    nrf->reg[SETUP_AW] = nrf24l01_reg_read(nrf, SETUP_AW);
    nrf->reg[SETUP_RETR] = nrf24l01_reg_read(nrf, SETUP_RETR);
    nrf->reg[RF_CH] = nrf24l01_reg_read(nrf, RF_CH);
    nrf->reg[RF_SETUP] = nrf24l01_reg_read(nrf, RF_SETUP);
    nrf->reg[STATUS] = nrf24l01_reg_read(nrf, STATUS);

    nrf->chip_enable(NRF24L01CE_ENABLE); // CE为高。设置RX时，进入接收模式；设置为TX时,10us后启动发送
}

/*检测24L01是否存在,返回值:0，成功;1，失败*/
static uint8_t nrf24l01_check(nrf24l01_t *nrf)
{
    uint8_t writeBuf[5] = {0XA5, 0XA5, 0XA5, 0XA5, 0XA5};
    uint8_t readBuf[5] = {0XAA, 0XAA, 0XAA, 0XAA, 0XAA};
    uint8_t status = 0;

    nrf24l01_buf_write(nrf, WRITE_REG_NRF + TX_ADDR, writeBuf, 5); /*写入5个字节的地址*/
    nrf24l01_buf_read(nrf, TX_ADDR, readBuf, 5);                   /*读出写入的地址*/

    for (int i = 0; i < 5; i++)
    {
        if (readBuf[i] != 0XA5)
        {
            status = 1; // 检测nRF24L01错误
            break;
        }
    }

    return status;
}

/*写寄存器*/
/*参数：reg:指定寄存器地址*/
/*      value:写入的值*/
/*返回值：状态值*/
static uint8_t nrf24l01_reg_write(nrf24l01_t *nrf, uint8_t reg, uint8_t value)
{
    uint8_t status;

    nrf->chip_select(NRF24L01CS_ENABLE);  // 使能SPI传输
    status = nrf->byte_readwrite(reg);    // 发送寄存器号
    nrf->byte_readwrite(value);           // 写入寄存器的值
    nrf->chip_select(NRF24L01CS_DISABLE); // 禁止SPI传输

    return (status); // 返回状态值
}

/*读取寄存器值*/
/*参数：reg:要读的寄存器*/
/*返回值：读取的寄存器值*/
static uint8_t nrf24l01_reg_read(nrf24l01_t *nrf, uint8_t reg)
{
    uint8_t reg_val;

    nrf->chip_select(NRF24L01CS_ENABLE); // 使能SPI传输

    nrf->byte_readwrite(reg);             // 发送寄存器号
    reg_val = nrf->byte_readwrite(0XFF);  // 读取寄存器内容
    nrf->chip_select(NRF24L01CS_DISABLE); // 禁止SPI传输

    return (reg_val); // 返回状态值
}

/*在指定位置读出指定长度的数据*/
/*参数：reg:寄存器(位置)*/
/*      *pBuf:数据指针*/
/*      len:数据长度*/
/*返回值,此次读到的状态寄存器值*/
static uint8_t nrf24l01_buf_read(nrf24l01_t *nrf, uint8_t reg, uint8_t *pBuf, uint8_t len)
{
    uint8_t status;

    nrf->chip_select(NRF24L01CS_ENABLE);

    status = nrf->byte_readwrite(reg); // 发送寄存器值(位置),并读取状态值

    for (int i = 0; i < len; i++)
    {
        pBuf[i] = nrf->byte_readwrite(0XFF); // 读出数据
    }

    nrf->chip_select(NRF24L01CS_DISABLE);

    return status;
}

/*在指定位置写指定长度的数据*/
/*参数：reg:寄存器(位置)*/
/*      *pBuf:数据指针*/
/*      len:数据长度*/
/*返回值,此次读到的状态寄存器值*/
static uint8_t nrf24l01_buf_write(nrf24l01_t *nrf, uint8_t reg, uint8_t *pBuf, uint8_t len)
{
    uint8_t status;

    nrf->chip_select(NRF24L01CS_ENABLE); // 使能SPI传输

    status = nrf->byte_readwrite(reg); // 发送寄存器值(位置),并读取状态值

    for (int i = 0; i < len; i++)
    {
        nrf->byte_readwrite(pBuf[i]); // 写入数据
    }

    nrf->chip_select(NRF24L01CS_DISABLE); // 关闭SPI传输

    return status; // 返回读到的状态值
}

/*缺省片选处理函数*/
static void nrf24l01_cs_default(NRF24L01_CS_ET cs)
{
    // 用于在SPI通讯时，片选信号硬件电路选中的情况
    return;
}

/*********** (C) COPYRIGHT 1999-2018 Moonan Technology *********END OF FILE****/
