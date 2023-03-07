# nrf24l01寄存器
nRF24L01的命令格式：指令+数据,指令为高位在前，
数据是低字节在前，具体指令如下

## 指令地址
| 指令名称     | 指令格式 | 实现操作                    |
| :----------- | :------- | :-------------------------- |
| R_REGISTER   | 000AAAAA | 读寄存器，AAAAA为寄存器地址 |
| W_REGISTER   | 001AAAAA | 写寄存器，AAAAA为寄存器地址 |
| R_RX_PAYLOAD | 01100001 | 读RX有效数据，在RX模式有效  |
| W_TX_PAYLOAD | 10100000 | 写TX有效数据，在TX模式有效  |
| FLUSH_TX     | 11100001 | 清除TX_FIFO寄存器           |
| FLUSH_RX     | 11100010 | 清楚RX_FIFO寄存器           |
| REUSE_TX_PL  | 11100011 | 重新使用上一包有效数据      |
| NOP          | 11111111 | 空操作，可用来读状态寄存器  |

## 寄存器及对象地址如下：                                 

| 地址(Hex) | 名称        | 属性 | 说明                        |
| :-------- | :---------- | :--- | :-------------------------- |
| 00        | CONFIG      | 读写 | 配置寄存器                  |
| 01        | EN_AA       | 读写 | 使能 自动应答 功能          |
| 02        | EN_RXADDR   | 读写 | 接收地址允许                |
| 03        | SETUP_AW    | 读写 | 设置地址宽度,所有数据通道   |
| 04        | SETUP_RETR  | 读写 | 建立自动重发                |
| 05        | RF_CH       | 读写 | 射频通道                    |
| 06        | RF_SETUP    | 读写 | 射频寄存器                  |
| 07        | STATUS      | 读写 | 状态寄存器                  |
| 08        | OBSERVE_TX  | 只读 | 发送检测寄存器              |
| 09        | CD          | 只读 | 载波检测                    |
| 0A        | RX_ADDR_P0  | 读写 | 数据通道 0 接收地址         |
| 0B        | RX_ADDR_P1  | 读写 | 数据通道 1 接收地址         |
| 0C        | RX_ADDR_P2  | 读写 | 数据通道 2 接收地址         |
| 0D        | RX_ADDR_P3  | 读写 | 数据通道 3 接收地址         |
| 0E        | RX_ADDR_P4  | 读写 | 数据通道 4 接收地址         |
| 0F        | RX_ADDR_P5  | 读写 | 数据通道 5 接收地址         |
| 10        | TX_ADDR     | 读写 | 发送地址                    |
| 11        | RX_PW_P0    | 读写 | 接收数据通道 0 有效数据宽度 |
| 12        | RX_PW_P1    | 读写 | 接收数据通道 1 有效数据宽度 |
| 13        | RX_PW_P2    | 读写 | 接收数据通道 2 有效数据宽度 |
| 14        | RX_PW_P3    | 读写 | 接收数据通道 3 有效数据宽度 |
| 15        | RX_PW_P4    | 读写 | 接收数据通道 4 有效数据宽度 |
| 16        | RX_PW_P5    | 读写 | 接收数据通道 5 有效数据宽度 |
| 17        | FIFO_STATUS | 只读 | FIFO 状态寄存器             |
| N/A       | TX_PLD      | 只写 | TXdata payload register     |
| N/A       | RX_PLD      | 只读 | RX data payload register    |

# 使用模板

```c
static nrf24l01_t nrf;

static uint8_t NRF24L01ReadWriteByte(uint8_t txdata); // 声明向nRF24L01读写一个字节的函数
static void NRF24L01ChipSelect(NRF24L01_CS_ET cs);    // 声明片选操作函数
static void NRF24L01ChipEnable(NRF24L01_CE_ET en);    // 声明使能及模式操作函数
static uint8_t NRF24L01GetIRQ(void);                  // 声明中断获取函数
static void NRF24L01Delayms(volatile uint32_t ntime); // 毫秒延时操作指针

/**
 * @brief   nrf 对象初始化
 *
 */
void nrf24l01_init(void)
{
    if (NRF24L01_NOERROR != nrf24L01_init(&nrf,
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
static void NRF24L01ChipSelect(NRF24L01_CS_ET cs)
{
    bsp_pin_write(get_pinnum(NRF_CS_PIN), cs);
}
static void NRF24L01ChipEnable(NRF24L01_CE_ET en) // ce
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

/**
 * @brief   nrf 收发一体化函数
 *
 */
void NRF24L01DataExchange(void)
{
    uint8_t txDatas[32] = {0x0f,0xf0,0xaa,0xf0,0x0f};// 这个波形会很有意思
    uint8_t rxDatas[32] = {0x00};

    // 发送
    if (0x20 == nrf24l01_packet_xmit(&nrf, txDatas))
    {
        printf("send success\r\n");
    }
    else
    {
        printf("send failed\r\n");
    }
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
```