/*
*********************************************************************************************************
*
*	模块名称 : SPI总线驱动
*	文件名称 : bsp_spi_bus.h
*
*********************************************************************************************************
*/

#include "board.h"
#include "bsp_spi.h"

uint8_t g_spi_busy = 0; /* SPI 总线共享标志 */
/*
*********************************************************************************************************
*	函 数 名: bsp_initspibus
*	功能说明: 配置SPI总线。 只包括 SCK、 MOSI、 MISO口线的配置。不包括片选CS，也不包括外设芯片特有的INT、BUSY等
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_initspibus(void)
{
#ifdef SOFT_SPI /* 软件SPI */
    GPIO_InitTypeDef GPIO_InitStructure;

    /* 打开GPIO时钟 */
    SPI_SCK_PORT_ENABLE();
    SPI_MOSI_PORT_ENABLE();
    SPI_MISO_PORT_ENABLE();

    /* 配置几个推完输出IO */
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;   /* 设为输出口 */
    GPIO_InitStructure.Pin = PIN_SCK;                /* 设为推挽模式 */
    GPIO_InitStructure.Pull = GPIO_PULLUP;           /* 上下拉电阻不使能 */
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH; /* IO口最大速度 */
    HAL_GPIO_Init(PORT_SCK, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = PIN_MOSI;
    HAL_GPIO_Init(PORT_MOSI, &GPIO_InitStructure);

    /* 配置GPIO为浮动输入模式(实际上CPU复位后就是输入状态) */
    GPIO_InitStructure.Mode = GPIO_MODE_INPUT; /* 设为输入口 */
    GPIO_InitStructure.Pin = PIN_MISO;
    GPIO_InitStructure.Pull = GPIO_NOPULL;           /* 无需上下拉电阻 */
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH; /* IO口最大速度 */
    HAL_GPIO_Init(PORT_MISO, &GPIO_InitStructure);
#endif

#ifdef HARD_SPI
    /* 硬件SPI */
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* 开启GPIO时钟 */
    __HAL_RCC_SPI1_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**SPI1 GPIO Configuration
    PB3     ------> SPI1_SCK
    PB4     ------> SPI1_MISO
    PB5     ------> SPI1_MOSI
    */
    GPIO_InitStruct.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    bsp_spi_init();

    /* Activate the SPI mode (Reset I2SMOD bit in I2SCFGR register) */
    SPI_HARD.Instance->I2SCFGR &= SPI_Mode_Select; /* 选择SPI模式，不是I2S模式 */

    /*---------------------------- SPIx CRCPOLY Configuration --------------------*/
    __HAL_SPI_DISABLE(&SPI_HARD);

    __HAL_SPI_ENABLE(&SPI_HARD);

#endif
}

/*
*********************************************************************************************************
*	函 数 名: bsp_spi_init
*	功能说明: 配置STM32内部SPI硬件的工作模式。 简化库函数，提高执行效率。 仅用于SPI接口间切换。
*	形    参: _cr1 寄存器值
*	返 回 值: 无
*********************************************************************************************************
*/
#ifdef HARD_SPI /* 硬件SPI */
void bsp_spi_init()
{
    SPI_HARD.Instance = SPI1;
    SPI_HARD.Init.Mode = SPI_MODE_MASTER;
    SPI_HARD.Init.Direction = SPI_DIRECTION_2LINES;
    SPI_HARD.Init.DataSize = SPI_DATASIZE_8BIT;
    SPI_HARD.Init.CLKPolarity = SPI_POLARITY_LOW;
    SPI_HARD.Init.CLKPhase = SPI_PHASE_1EDGE;
    SPI_HARD.Init.NSS = SPI_NSS_SOFT;
    SPI_HARD.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
    SPI_HARD.Init.FirstBit = SPI_FIRSTBIT_MSB;
    SPI_HARD.Init.TIMode = SPI_TIMODE_DISABLE;
    SPI_HARD.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    SPI_HARD.Init.CRCPolynomial = 10;
    if (HAL_SPI_Init(&SPI_HARD) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN SPI1_Init 2 */
    __HAL_SPI_ENABLE(&SPI_HARD);
    /* USER CODE END SPI1_Init 2 */
}
#endif

#ifdef SOFT_SPI /* 软件SPI */
/*
*********************************************************************************************************
*	函 数 名: bsp_SpiDelay
*	功能说明: 时序延迟
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_spiDelay(void)
{
    uint32_t i;
    for (i = 0; i < 5; i++)
        ;
}
#endif

/*
*********************************************************************************************************
*	函 数 名: bsp_spiwrite
*	功能说明: 向SPI总线发送一个字节。SCK上升沿采集数据, SCK空闲时为低电平。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_spiwrite0(uint8_t byte)
{
#ifdef SOFT_SPI /* 软件SPI */
    uint8_t i;

    for (i = 0; i < 8; i++)
    {
        if (byte & 0x80)
        {
            MOSI_1();
        }
        else
        {
            MOSI_0();
        }
        bsp_spiDelay();
        SCK_1();
        byte <<= 1;
        bsp_spiDelay();
        SCK_0();
    }
    bsp_spiDelay();
#endif

#ifdef HARD_SPI /* 硬件SPI */
    HAL_SPI_Transmit(&SPI_HARD, &byte, 1, 1000);
#endif
}

/*
*********************************************************************************************************
*	函 数 名: bsp_spiread0
*	功能说明: 从SPI总线接收8个bit数据。 SCK上升沿采集数据, SCK空闲时为低电平。
*	形    参: 无
*	返 回 值: 读到的数据
*********************************************************************************************************
*/
uint8_t bsp_spiread0(void)
{
#ifdef SOFT_SPI /* 软件SPI */
    uint8_t i;
    uint8_t read = 0;

    for (i = 0; i < 8; i++)
    {
        read = read << 1;

        if (MISO_IS_HIGH())
        {
            read++;
        }
        SCK_1();
        bsp_spiDelay();
        SCK_0();
        bsp_spiDelay();
    }
    return read;
#endif

#ifdef HARD_SPI /* 硬件SPI */
    uint8_t read;
    HAL_SPI_Receive(&SPI_HARD, &read, 1, 1000);
    return read;
#endif
}

uint8_t bsp_spi_readwrite0(uint8_t _ucByte)
{
#ifdef SOFT_SPI /* 软件SPI */
    uint8_t i;

    for (i = 0; i < 8; i++)
    {
        if (byte & 0x80)
        {
            MOSI_1();
        }
        else
        {
            MOSI_0();
        }
        bsp_spiDelay();
        read = read << 1;

        if (MISO_IS_HIGH())
        {
            read++;
        }
        SCK_1();
        byte <<= 1;
        bsp_spiDelay();
        SCK_0();
    }
    bsp_spiDelay();
    return read;
#endif

#ifdef HARD_SPI /* 硬件SPI */
    uint8_t recv = 0;
    HAL_SPI_TransmitReceive(&SPI_HARD, &_ucByte, &recv, 1, 1000);
    return recv;
#endif
}

/*
*********************************************************************************************************
*	函 数 名: bsp_spiWrite1
*	功能说明: 向SPI总线发送一个字节。  SCK上升沿采集数据, SCK空闲时为高电平
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_spiwrite1(uint8_t _ucByte)
{
#ifdef SOFT_SPI /* 软件SPI */
    uint8_t i;

    for (i = 0; i < 8; i++)
    {
        if (_ucByte & 0x80)
        {
            MOSI_1();
        }
        else
        {
            MOSI_0();
        }
        SCK_0();
        _ucByte <<= 1;
        bsp_spiDelay();
        SCK_1(); /* SCK上升沿采集数据, SCK空闲时为高电平 */
        bsp_spiDelay();
    }
#endif

#ifdef HARD_SPI /* 硬件SPI */
    HAL_SPI_Transmit(&SPI_HARD, &_ucByte, 1, 1000);
#endif
}

uint8_t bsp_spi_readwrite1(uint8_t _ucByte)
{
#ifdef SOFT_SPI /* 软件SPI */
    uint8_t i;
    uint8_t read = 0;

    for (i = 0; i < 8; i++)
    {
        if (_ucByte & 0x80)
        {
            MOSI_1();
        }
        else
        {
            MOSI_0();
        }
        SCK_0();
        _ucByte <<= 1;
        bsp_spiDelay();
        read = read << 1;
        if (MISO_IS_HIGH())
        {
            read++;
        }
        SCK_1(); /* SCK上升沿采集数据, SCK空闲时为高电平 */
        bsp_spiDelay();
    }
    return read;
#endif

#ifdef HARD_SPI /* 硬件SPI */
    uint8_t recv = 0;
    HAL_SPI_TransmitReceive(&SPI_HARD, &_ucByte, &recv, 1, 1000);
    return recv;
#endif
}

/*
*********************************************************************************************************
*	函 数 名: bsp_spiRead1
*	功能说明: 从SPI总线接收8个bit数据。  SCK上升沿采集数据, SCK空闲时为高电平
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
uint8_t bsp_spiread1(void)
{
#ifdef SOFT_SPI /* 软件SPI */
    uint8_t i;
    uint8_t read = 0;

    for (i = 0; i < 8; i++)
    {
        SCK_0();
        bsp_spiDelay();
        read = read << 1;
        if (MISO_IS_HIGH())
        {
            read++;
        }
        SCK_1();
        bsp_spiDelay();
    }
    return read;
#endif

#ifdef HARD_SPI /* 硬件SPI */
    uint8_t read;
    HAL_SPI_Receive(&SPI_HARD, &read, 1, 1000);
    return read;
#endif
}

/*
*********************************************************************************************************
*	函 数 名: bsp_SpiBusEnter
*	功能说明: 占用SPI总线
*	形    参: 无
*	返 回 值: 0 表示不忙  1表示忙
*********************************************************************************************************
*/
void bsp_spibusenter(void)
{
    g_spi_busy = 1;
}

/*
*********************************************************************************************************
*	函 数 名: bsp_SpiBusExit
*	功能说明: 释放占用的SPI总线
*	形    参: 无
*	返 回 值: 0 表示不忙  1表示忙
*********************************************************************************************************
*/
void bsp_spibusexit(void)
{
    g_spi_busy = 0;
}

/*
*********************************************************************************************************
*	函 数 名: bsp_spibusbusy
*	功能说明: 判断SPI总线忙。方法是检测其他SPI芯片的片选信号是否为1
*	形    参: 无
*	返 回 值: 0 表示不忙  1表示忙
*********************************************************************************************************
*/
uint8_t bsp_spibusbusy(void)
{
    return g_spi_busy;
}

/*
*********************************************************************************************************
*	函 数 名: bsp_setspisck
*	功能说明: 用于软件模式。设置SCK GPIO的状态。在函数CS=0之前被调用，用于不同相序的SPI设备间切换。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
#ifdef SOFT_SPI /* 软件SPI */
void bsp_setspisck(uint8_t _data)
{
    if (_data == 0)
    {
        SCK_0();
    }
    else
    {
        SCK_1();
    }
}
#endif
